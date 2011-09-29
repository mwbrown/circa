// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "building.h"
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

void for_loop_start_iteration(EvalContext* context, Term* caller);
void for_loop_complete_empty_list(EvalContext* context, Term* caller);

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
    Branch* attributes = create_branch_unevaluated(forContents, "#attributes");
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
    Branch* outerRebinds = create_branch_unevaluated(forContents, "#outer_rebinds");

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
#if 0
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
#endif
}

void for_loop_begin_branch(EvalContext* context)
{
    Term* caller = get_pc_term(context);
    ca_assert(caller->function == FOR_FUNC);
    Branch* contents = nested_contents(caller);

    // Initialize output
    List* inputList = as_list(get_current_input(context, 0));
    /*List* output =*/ set_list(get_current_output(context), inputList->length());

    // Check if this is an empty list. If so, there is a special path for copying rebinds
    if (inputList->length() == 0) {
        for_loop_complete_empty_list(context, caller);
        return;
    }

    // Create stack frame
    Frame* frame = push_frame(context, contents);
    frame->overrideFinishBranch = for_loop_finish_iteration;

    // Copy inner rebinds for 1st iteration
    {
        int index = inner_rebinds_location;
        while (contents->get(index)->function == JOIN_FUNC) {
            Term* rebindTerm = contents->get(index);
            TaggedValue* dest = get_output2(context, rebindTerm);

            copy(get_input2(context, rebindTerm, 0), dest);
            index++;
        }
    }

    // Initialize index term
    Term* indexTerm = contents->get(index_location);
    set_int(get_output2(context, indexTerm), 0);

    // Begin the loop proper
    for_loop_start_iteration(context, caller);

#if 0
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
#endif
}

// This is called before starting the 2nd or later iterations
void for_loop_prepare_another_iteration(EvalContext* context, Term* caller)
{
    Branch* contents = nested_contents(caller);

    // Copy inner rebinds
    int index = inner_rebinds_location;
    while (contents->get(index)->function == JOIN_FUNC) {
        Term* rebindTerm = contents->get(index);
        TaggedValue* dest = get_output2(context, rebindTerm);

        copy(get_input2(context, rebindTerm, 1), dest);
        index++;
    }
}

void for_loop_start_iteration(EvalContext* context, Term* caller)
{
    Branch* contents = nested_contents(caller);

    // Fetch current iterator value
    Term* indexTerm = contents->get(index_location);
    int indexValue = as_int(get_output2(context, indexTerm));
    List* inputList = as_list(get_input2_rel(context, caller, 1, 0));

    Term* iteratorTerm = contents->get(iterator_location);
    copy(inputList->get(indexValue), get_output2(context, iteratorTerm));

    // Fetch local state
    set_dict(&get_frame(context,0)->state);
    Dict* outsideState = &get_frame(context, 1)->state;
    TaggedValue* stateEntry = outsideState->get(get_unique_name(caller));
    if (stateEntry != NULL) {
        List* stateList = List::lazyCast(stateEntry);
        if (indexValue < stateList->length()) {
            TaggedValue* entry = stateList->get(indexValue);
            if (!is_dict(entry))
                set_dict(entry);
            move(entry, &get_frame(context, 0)->state);
        }
    }

    // Skip PC past some metadata
    top_frame(context)->pc = iterator_location+1;
}

void for_loop_complete_empty_list(EvalContext* context, Term* caller)
{
    top_frame(context)->pc++;

    // Copy outer rebinds
    Branch* outerRebinds = get_for_loop_outer_rebinds(caller);
    for (int i=0; i < outerRebinds->length(); i++) {

        Term* rebindTerm = outerRebinds->get(i);

        TaggedValue* result = get_input2_rel(context, rebindTerm, -1, 0);
        copy(result, get_extra_output2(context, caller, i));
    }
}

bool for_loop_finish_iteration(EvalContext* context, int flags)
{
    bool exitLoop = (flags & 1) != 0;
    Branch* contents = top_frame(context)->branch;
    Term* caller = contents->owningTerm;
    ca_assert(caller->function == FOR_FUNC);

    // Fetch index
    Term* indexTerm = contents->get(index_location);
    TaggedValue* indexValue = get_output2(context, indexTerm);
    int index = as_int(indexValue);

    // Save output for this iteration
    Term* listResultTerm = NULL;
    if (as_bool(get_for_loop_modify_list(caller)))
        listResultTerm = contents->get(get_for_loop_iterator(caller)->name);
    else
        listResultTerm = find_last_non_comment_expression(contents);
    TaggedValue* listResult = get_output2(context, listResultTerm);
    list_set_index(get_output2_rel(context, caller, 1), index, listResult);

    // Save local state
    Dict* prevScope = &get_frame(context, 1)->state;
    List* stateList = List::lazyCast(prevScope->insert(get_unique_name(caller)));

    if (stateList->length() <= index)
        stateList->resize(index + 1);

    swap(&get_frame(context, 0)->state, stateList->get(index));

    // Increment iterator index
    set_int(indexValue, index + 1);

    // Check if we have completed the loop
    List* inputList = as_list(get_input2_rel(context, caller, 1, 0));

    if (!exitLoop && (as_int(indexValue) < inputList->length())) {
        // Start a new iteration
        for_loop_prepare_another_iteration(context, caller);
        for_loop_start_iteration(context, caller);
        return false;
    }

    // Finish the loop
    
    // Copy external rebinds
    Branch* outerRebinds = get_for_loop_outer_rebinds(caller);
    for (int i=0; i < outerRebinds->length(); i++) {

        Term* rebindTerm = outerRebinds->get(i);
        TaggedValue* result = get_input2(context, rebindTerm, 1);
        TaggedValue* dest = get_extra_output2_rel(context, caller, 1, i);
        copy(result, dest);
    }

    return true;
}

void for_loop_break(EvalContext* context)
{
    while (true) {
        Branch* branch = get_frame(context, 0)->branch;
        Term* caller = branch->owningTerm;
        if (caller->function == FOR_FUNC)
            break;
        finish_branch(context, 0);
    }
    finish_branch(context, 1);
}

void for_loop_continue(EvalContext* context)
{
    while (true) {
        Branch* branch = get_frame(context, 0)->branch;
        Term* caller = branch->owningTerm;
        if (caller->function == FOR_FUNC)
            break;
        finish_branch(context, 0);
    }
    finish_branch(context, 0);
}

} // namespace circa
