// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "function.h"
#include "introspection.h"
#include "term.h"

#include "locals.h"

namespace circa {

int get_output_count(Term* term)
{
    if (!FINISHED_BOOTSTRAP)
        return 1;

    // check if the function has overridden getOutputCount
    FunctionAttrs::GetOutputCount getOutputCount = NULL;

    if (term->function == NULL)
        return 1;

    FunctionAttrs* attrs = get_function_attrs(term->function);

    if (attrs == NULL)
        return 1;
    
    getOutputCount = attrs->getOutputCount;

    if (getOutputCount != NULL)
        return getOutputCount(term);

    // Default behavior, if FunctionAttrs was found.
    return attrs->outputCount;
}
    
void update_locals_index_for_new_term(Term* term)
{
    Branch* branch = term->owningBranch;

    term->outputCount = get_output_count(term);

    // make sure localsIndex is -1 so that if get_locals_count looks at this
    // term, it doesn't get confused.
    term->localsIndex = -1;
    if (term->outputCount > 0)
        term->localsIndex = get_locals_count(*branch);
}

int get_locals_count(Branch& branch)
{
    if (branch.length() == 0)
        return 0;

    int lastLocal = branch.length() - 1;

    while (branch[lastLocal] == NULL || branch[lastLocal]->localsIndex == -1) {
        lastLocal--;
        if (lastLocal < 0)
            return 0;
    }

    Term* last = branch[lastLocal];

    return last->localsIndex + get_output_count(last);
}

void refresh_locals_indices(Branch& branch, int startingAt)
{
    int nextLocal = 0;
    if (startingAt > 0) {
        Term* prev = branch[startingAt - 1];
        nextLocal = prev->localsIndex + get_output_count(prev);
    }

    for (int i=startingAt; i < branch.length(); i++) {
        Term* term = branch[i];
        if (term == NULL)
            continue;
        term->localsIndex = nextLocal;

        int newOutputCount = get_output_count(term);
        if (term->outputCount != newOutputCount) {
            term->outputCount = newOutputCount;
            update_input_instructions(term);
        }

        term->outputCount = get_output_count(term);
        nextLocal += term->outputCount;
    }
}


void update_output_count(Term* term)
{
    term->outputCount = get_output_count(term);
}

bool branch_creates_separate_stack_frame(Branch* branch)
{
    if (branch->owningTerm == NULL)
        return true;

    if (branch->owningTerm->name == "#inner_rebinds"
            || branch->owningTerm->name == "#outer_rebinds")
        return false;

    if (branch->owningTerm->function == IF_BLOCK_FUNC)
        return false;

    return true;
}

int get_frame_distance(Term* term, Term* input)
{
    if (input == NULL)
        return -1;

    Branch* inputBranch = input->owningBranch;

    // Special case for if_block. Terms inside #joining can see terms inside each case.
    Term* termParent = get_parent_term(term);
    if (termParent != NULL
            && termParent->name == "#joining"
            && termParent->owningBranch == get_parent_branch(*inputBranch))
        return 0;

    // If the input's branch doesn't create a separate stack frame, then look
    // at the parent branch.
    if (!branch_creates_separate_stack_frame(inputBranch))
        inputBranch = get_parent_branch(*inputBranch);

    Branch* fromBranch = term->owningBranch;

    // Walk upward from 'term' until we find the common branch.
    int distance = 0;
    while (fromBranch != inputBranch) {

        if (branch_creates_separate_stack_frame(fromBranch))
            distance++;

        fromBranch = get_parent_branch(*fromBranch);

        if (fromBranch == NULL)
            return -1;
    }
    return distance;
}

void update_input_instructions(Term* term)
{
    InputInstructionList& list = term->inputIsns;

    list.inputs.resize(term->numInputs());
    for (int i=0; i < term->numInputs(); i++) {
        Term* input = term->input(i);
        if (input == NULL) {
            list.inputs[i].type = InputInstruction::EMPTY;
            continue;
        }

        if (is_value(input)) {
            list.inputs[i].type = InputInstruction::GLOBAL;
            continue;
        }

        list.inputs[i].type = InputInstruction::LOCAL;
        list.inputs[i].relativeFrame = get_frame_distance(term, input);
        list.inputs[i].index = input->localsIndex + term->inputInfo(i)->outputIndex;
        //ca_assert(list.inputs[i].relativeFrame >= 0);
        
        // Fun special case for for-loop locals
        if (input->function == JOIN_FUNC && get_parent_term(input)->name == "#inner_rebinds")
            list.inputs[i].index = 1 + input->localsIndex;
    }
}

void update_input_instructions(Branch& branch)
{
    for (int i=0; i < branch.length(); i++)
        update_input_instructions(branch[i]);
}

} // namespace circa
