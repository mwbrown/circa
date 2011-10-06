// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "building.h"
#include "bytecode.h"
#include "evaluation.h"
#include "importing_macros.h"
#include "interpreter.h"
#include "introspection.h"
#include "kernel.h"
#include "list_shared.h"
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
   [0] iterator
   [1..i] join() terms for inner rebinds
   [...] contents
   [n-1] #outer_rebinds
*/

static const int iterator_location = 0;
static const int inner_rebinds_location = 1;

Term* for_loop_get_iterator(Term* forTerm)
{
    return forTerm->contents(iterator_location);
}

Branch* get_for_loop_outer_rebinds(Term* forTerm)
{
    Branch* contents = nested_contents(forTerm);
    return contents->getFromEnd(0)->contents();
}

void setup_for_loop_pre_code(Term* forTerm)
{
    Branch* contents = nested_contents(forTerm);

    // Reserve 2 locals, used during execution
    reserve_local_value(contents);
    reserve_local_value(contents);

    // Create iterator term
    Type* iteratorType = infer_type_of_get_index(forTerm->input(0));
    Term* result = apply(nested_contents(forTerm), INPUT_PLACEHOLDER_FUNC, TermList());
    change_declared_type(result, iteratorType);
    hide_from_source(result);
    ca_assert(result->index == iterator_location);
}

void for_loop_rename_iterator(Term* forTerm, const char* name)
{
    Term* term = forTerm->contents(0);
    nested_contents(forTerm)->bindName(term, name);
}

void setup_for_loop_post_code(Term* forTerm)
{
    Branch* forContents = nested_contents(forTerm);
    std::string listName = forTerm->input(0)->name;
    std::string iteratorName = for_loop_get_iterator(forTerm)->name;

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

void for_block_write_bytecode(Term* caller, BytecodeWriter* writer)
{
    bc_push_branch(writer, caller);
}

void for_block_write_bytecode_contents(Term* caller, BytecodeWriter* writer)
{
    Branch* contents = nested_contents(caller);
    Branch* parent = caller->owningBranch;
    // bool useState = has_any_inlined_state(contents);
    Branch* outerRebinds = get_for_loop_outer_rebinds(caller);
    bool anyOuterRebinds = outerRebinds->length() > 0;

    const int indexLocal = 0;
    const int lengthLocal = 1;

    // Start index with 0
    bc_assign_local(writer, indexLocal);
    bc_int_input(writer, 0);

    // Fetch list length
    bc_write_call(writer, get_global("length"));
    bc_local_input(writer, lengthLocal);
    bc_write_input(writer, contents, caller->input(0));

    // Prepare output value.
    bc_write_call(writer, get_global("blank_list"));
    bc_write_input(writer, contents, caller);
    bc_local_input(writer, lengthLocal);

    // Check if we are already finished (ie, iterating over an empty list)
    int jumpPastEmptyList = bc_jump_if_less_than(writer);
    bc_local_input(writer, indexLocal);
    bc_local_input(writer, lengthLocal);

    // This part is evaluated when iterating over an empty list. Copy joined locals
    // appropriately.
    for (int i=0; i < outerRebinds->length(); i++) {
        bc_copy(writer);
        bc_write_input(writer, contents, outerRebinds->get(i)->input(0));
        bc_local_input(writer, 1, parent->get(caller->index + 1 + i)->local);
    }

    // Finish branch for empty list
    bc_pop_branch(writer);

    // Setup for first iteration. Copy inner rebinds from their outside sources.
    bc_jump_to_here(writer, jumpPastEmptyList);

    for (int i=inner_rebinds_location; contents->get(i)->function == JOIN_FUNC; i++) {
        Term* term = contents->get(i);
        bc_copy(writer);
        bc_write_input(writer, contents, term->input(0));
        bc_write_input(writer, contents, term);
    }

    int jumpPast2ndIterationSetup = 0;
    if (anyOuterRebinds)
        jumpPast2ndIterationSetup = bc_jump(writer);

    // For 2nd and later iterations, copy inner rebinds from the bottom of the loop.
    int secondIterationStart = writer->writePosition;

    for (int i=inner_rebinds_location; contents->get(i)->function == JOIN_FUNC; i++) {
        Term* term = contents->get(i);
        bc_copy(writer);
        bc_write_input(writer, contents, term->input(1));
        bc_write_input(writer, contents, term);
    }

    if (anyOuterRebinds)
        bc_jump_to_here(writer, jumpPast2ndIterationSetup);

    // Fetch iterator value.
    Term* iterator = contents->get(iterator_location);
    bc_write_call(writer, get_global("get_index"));
    bc_write_input(writer, contents, iterator);
    bc_write_input(writer, contents, caller->input(0));
    bc_local_input(writer, indexLocal);

    // Loop contents
    for (int i = inner_rebinds_location; i < contents->length() - 1; i++)
        bc_call(writer, contents->get(i));

    // Save the list result
    Term* listResult = NULL;
    if (caller->boolPropOptional("modifyList", false))
        listResult = contents->get(for_loop_get_iterator(caller)->name);
    else
        listResult = find_last_non_comment_expression(contents);
        
    bc_write_call(writer, SET_INDEX_FUNC);
    bc_write_input(writer, contents, caller);
    bc_write_input(writer, contents, caller);
    bc_local_input(writer, indexLocal);
    bc_write_input(writer, contents, listResult);

    // Finish iteration, increment index.
    bc_increment(writer);
    bc_local_input(writer, indexLocal);

    // Jump back to the start of the loop (if we haven't reached the end).
    bc_jump_to_pos(writer, bc_jump_if_less_than(writer), secondIterationStart);
    bc_local_input(writer, indexLocal);
    bc_local_input(writer, lengthLocal);

    // Finished looping, copy outer rebinds
    for (int i=0; i < outerRebinds->length(); i++) {
        bc_copy(writer);
        bc_write_input(writer, contents, outerRebinds->get(i)->input(1));
        bc_local_input(writer, 1, parent->get(caller->index + 1 + i)->local);
    }

    // End
    bc_pop_branch(writer);
}

} // namespace circa
