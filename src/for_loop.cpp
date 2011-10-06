// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "building.h"
#include "bytecode.h"
#include "evaluation.h"
#include "importing_macros.h"
#include "introspection.h"
#include "kernel.h"
#include "list_shared.h"
#include "locals.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "refactoring.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"
#include "update_cascades.h"

#include "for_loop.h"

namespace circa {

/* Organization of for loop contents:
   [0] #attributes
     [0] #modify_list
   [1] index
   [2] iterator
   [3..i] join() terms for inner rebinds
   [...] contents
   [n-1] #outer_rebinds
*/

static const int index_location = 1;
static const int iterator_location = 2;
static const int inner_rebinds_location = 3;

Term* get_for_loop_iterator(Term* forTerm)
{
    return forTerm->contents(iterator_location);
}

Term* get_for_loop_modify_list(Term* forTerm)
{
    Term* term = forTerm->contents(0)->contents(0);
    ca_assert(term != NULL);
    return term;
}

Branch* get_for_loop_outer_rebinds(Term* forTerm)
{
    Branch* contents = nested_contents(forTerm);
    return contents->getFromEnd(0)->contents();
}

void setup_for_loop_pre_code(Term* forTerm)
{
    Branch* forContents = nested_contents(forTerm);
    Branch* attributes = create_branch(forContents, "#attributes");
    create_bool(attributes, false, "#modify_list");
}

Term* setup_for_loop_iterator(Term* forTerm, const char* name)
{
    /*Term* indexTerm =*/ apply(nested_contents(forTerm), LOOP_INDEX_FUNC, TermList());
    Type* iteratorType = infer_type_of_get_index(forTerm->input(0));
    Term* result = apply(nested_contents(forTerm), INPUT_PLACEHOLDER_FUNC, TermList(), name);
    change_declared_type(result, iteratorType);
    hide_from_source(result);
    ca_assert(result->index == iterator_location);
    return result;
}

void setup_for_loop_post_code(Term* forTerm)
{
    Branch* forContents = nested_contents(forTerm);
    std::string listName = forTerm->input(0)->name;
    std::string iteratorName = get_for_loop_iterator(forTerm)->name;

    finish_minor_branch(forContents);

    // Create a branch that has all the names which are rebound in this loop
    Branch* outerRebinds = create_branch(forContents, "#outer_rebinds");

    std::vector<std::string> reboundNames;
    list_names_that_this_branch_rebinds(forContents, reboundNames);

    for (size_t i=0; i < reboundNames.size(); i++) {
        std::string const& name = reboundNames[i];
        if (name == listName)
            continue;
        if (name == iteratorName)
            continue;

        Term* original = get_named_at(forTerm, name);

        // The name might not be found, for certain parser errors.
        if (original == NULL)
            continue;

        Term* loopResult = forContents->get(name);

        // First input to both of these should be 'original', but we need to wait until
        // after remap_pointers before setting this.
        Term* innerRebind = apply(forContents, JOIN_FUNC, TermList(NULL, loopResult), name);

        change_declared_type(innerRebind, original->type);
        forContents->move(innerRebind, inner_rebinds_location + i);

        Term* outerRebind = apply(outerRebinds, JOIN_FUNC, TermList(NULL, loopResult), name);

        // Rewrite the loop code to use our local copies of these rebound variables.
        remap_pointers(forContents, original, innerRebind);

        set_input(innerRebind, 0, original);
        set_input(outerRebind, 0, original);

        respecialize_type(outerRebind);
    }

    for_loop_update_output_index(forTerm);
    update_input_instructions(forTerm);
    recursively_finish_update_cascade(forContents);
}

Term* find_enclosing_for_loop(Term* term)
{
    if (term == NULL)
        return NULL;

    if (term->function == FOR_FUNC)
        return term;

    Branch* branch = term->owningBranch;
    if (branch == NULL)
        return NULL;

    return find_enclosing_for_loop(branch->owningTerm);
}

void for_loop_update_output_index(Term* forTerm)
{
    Branch* contents = nested_contents(forTerm);

    // If this is a list-rewrite, then the output is the last term that has the iterator's
    // name binding. Otherwise the output is the last expression.
    if (as_bool(get_for_loop_modify_list(forTerm))) {
        Term* output = contents->get(get_for_loop_iterator(forTerm)->name);
        ca_assert(output != NULL);
        contents->outputIndex = output->index;
    } else {
        // Find the first non-comment expression before #outer_rebinds
        Term* output = find_last_non_comment_expression(contents);
        contents->outputIndex = output == NULL ? -1 : output->index;
    }
}

CA_FUNCTION(evaluate_for_loop)
{
    Term* caller = CALLER;
    EvalContext* context = CONTEXT;
    Branch* forContents = nested_contents(caller);
    Branch* outerRebinds = get_for_loop_outer_rebinds(caller);
    Term* iterator = get_for_loop_iterator(caller);

    TaggedValue* inputList = INPUT(0);
    int inputListLength = inputList->numElements();

    TaggedValue outputTv;
    bool saveOutput = forContents->outputIndex != -1;
    List* output = set_list(&outputTv, inputListLength);
    int nextOutputIndex = 0;

    push_stack_frame(context, forContents);
    context->callStack.append(CALLER);

    // Prepare state container
    bool useState = has_implicit_state(CALLER);
    TaggedValue localState;
    List* state = NULL;
    if (useState) {
        push_scope_state(context);
        fetch_state_container(CALLER, get_scope_state(context, 1), &localState);

        state = List::lazyCast(&localState);
        state->resize(inputListLength);
    }

    // Preserve old for-loop context
    ForLoopContext prevLoopContext = context->forLoopContext;

    context->forLoopContext.breakCalled = false;

    for (int iteration=0; iteration < inputListLength; iteration++) {
        context->forLoopContext.continueCalled = false;

        bool firstIter = iteration == 0;

        // load state for this iteration
        if (useState)
            swap(state->get(iteration), get_scope_state(context, 0));

        // copy iterator
        copy(inputList->getIndex(iteration), get_local(context, 0, iterator));

        // copy inner rebinds
        {
            int index = inner_rebinds_location;
            while (forContents->get(index)->function == JOIN_FUNC) {
                Term* rebindTerm = forContents->get(index);
                TaggedValue* dest = get_local(context, 0, index);

                if (firstIter)
                    copy(get_input(context, rebindTerm, 0), dest);
                else
                    copy(get_input(context, rebindTerm, 1), dest);

                //ca_assert(cast_possible(dest, declared_type(rebindTerm)));

                index++;
            }
        }

        context->forLoopContext.discard = false;

        ca_assert(!evaluation_interrupted(context));

        evaluate_branch_with_bytecode(context, forContents);

        // Save output
        if (saveOutput && !context->forLoopContext.discard) {
            TaggedValue* localResult = get_local(context, 0, forContents->get(forContents->outputIndex));
            copy(localResult, output->get(nextOutputIndex++));
        }

        // Save state
        if (useState)
            swap(state->get(iteration), get_scope_state(context, 0));

        if (context->forLoopContext.breakCalled
                || context->interruptSubroutine)
            break;
    }

    // Resize output, in case some elements were discarded
    output->resize(nextOutputIndex);

    // Copy outer rebinds
    //ca_assert(caller->numOutputs() == outerRebinds.length() + 1);
    
    for (int i=0; i < outerRebinds->length(); i++) {

        Term* rebindTerm = outerRebinds->get(i);

        TaggedValue* result = NULL;

        if (inputListLength == 0) {
            // No iterations, use the outer rebind
            result = get_input(context, rebindTerm, 0);
        } else {
            // At least one iteration, use our local rebind
            result = get_input(context, rebindTerm, 1);
        }

        int outputIndex = caller->index + 1 + i;
        TaggedValue* dest = list_get_index(get_stack_frame(context, 1), outputIndex);
        copy(result, dest);
    }

    // Restore loop context
    context->forLoopContext = prevLoopContext;

    if (useState) {
        pop_scope_state(context);
        save_and_consume_state(CALLER, get_scope_state(context, 0), &localState);
    }

    context->callStack.pop();
    pop_stack_frame(context);
    
    // Copy output (need to do this after restoring stack)
    swap(output, OUTPUT);
}

void for_block_write_bytecode(Term* caller, BytecodeWriter* writer)
{
    bc_call_branch(writer, caller);
    bc_pop_stack(writer);
    bc_return_on_evaluation_interrupted(writer);
}

void for_block_write_bytecode_contents(Term* caller, BytecodeWriter* writer)
{
    Branch* contents = nested_contents(caller);
    Branch* parentBranch = caller->owningBranch;
    bool useState = has_any_inlined_state(contents);
    Branch* outerRebinds = get_for_loop_outer_rebinds(caller);
    Term* inputList = caller->input(0);

    Term* indexTerm = contents->get(index_location);
    ca_assert(indexTerm->function == LOOP_INDEX_FUNC);

    // Prepare output value.
    // FIXME
    //bc_write_call_op(writer, caller, get_global("loop_prepare_output"));
    //bc_write_input(writer, contents, inputList);

    // Loop setup. Write 0 to the index.
    bc_call(writer, indexTerm);
    
    // Check if we are already finished (ie, iterating over an empty list)
    int jumpPastEmptyList = bc_jump_if_within_range(writer);
    bc_write_input(writer, contents, inputList);
    bc_write_input(writer, contents, indexTerm);

    // These instructions are evaluated when iterating over an empty list. Copy
    // joined locals appropriately.
    for (int i=0; i < outerRebinds->length(); i++) {
        bc_copy_value(writer);
        bc_write_input(writer, contents, outerRebinds->get(i)->input(0));
        bc_local_input(writer, 1, caller->index + 1 + i);
    }

    int jumpToEnd = bc_jump(writer);

    // Setup for first iteration. Copy inner rebinds from their outside sources.
    bc_jump_to_here(writer, jumpPastEmptyList);

    for (int i=inner_rebinds_location; contents->get(i)->function == JOIN_FUNC; i++) {
        Term* term = contents->get(i);
        bc_copy_value(writer);
        bc_write_input(writer, contents, term->input(0));
        bc_write_input(writer, contents, term);
    }

    int jumpPast2ndIterationSetup = bc_jump(writer);

    // For 2nd and later iterations, copy inner rebinds from the bottom of the loop.
    int secondIterationStart = writer->writePosition;

    for (int i=inner_rebinds_location; contents->get(i)->function == JOIN_FUNC; i++) {
        Term* term = contents->get(i);
        bc_copy_value(writer);
        bc_write_input(writer, contents, term->input(1));
        bc_write_input(writer, contents, term);
    }

    bc_jump_to_here(writer, jumpPast2ndIterationSetup);

    // Fetch iterator value.
    Term* iterator = contents->get(iterator_location);
    bc_write_call_op_with_func(writer, iterator, GET_INDEX_FUNC);
    bc_write_input(writer, contents, inputList);
    bc_write_input(writer, contents, indexTerm);

    // Loop contents
    for (int i = inner_rebinds_location; i < contents->length() - 1; i++)
        bc_call(writer, contents->get(i));

    // Save the list result
    Term* listResult = NULL;
    if (as_bool(get_for_loop_modify_list(caller)))
        listResult = contents->get(get_for_loop_iterator(caller)->name);
    else
        listResult = find_last_non_comment_expression(contents);
        
    bc_write_call_op_with_func(writer, caller, SET_INDEX_FUNC);
    bc_write_input(writer, contents, caller);
    bc_write_input(writer, contents, indexTerm);
    bc_write_input(writer, contents, listResult);

    // Finish iteration, increment index.
    bc_increment(writer);
    bc_local_input(writer, 0, indexTerm->index);

    // Jump back to the start of the loop (if we haven't reached the end).
    int jumpToStart = bc_jump_if_within_range(writer);
    bc_write_input(writer, contents, inputList);
    bc_write_input(writer, contents, indexTerm);
    bc_jump_to_pos(writer, jumpToStart, secondIterationStart);

    // Finished looping, copy outer rebinds
    for (int i=0; i < outerRebinds->length(); i++) {
        bc_copy_value(writer);
        bc_write_input(writer, contents, outerRebinds->get(i)->input(1));
        bc_local_input(writer, 1, caller->index + 1 + i);
    }

    // End
    bc_jump_to_here(writer, jumpToEnd);
}

} // namespace circa
