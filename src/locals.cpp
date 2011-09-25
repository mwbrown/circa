// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "function.h"
#include "introspection.h"
#include "term.h"

#include "locals.h"

namespace circa {

int get_extra_output_count(Term* term)
{
    if (!FINISHED_BOOTSTRAP)
        return 0;

    // check if the function has overridden getOutputCount
    FunctionAttrs::GetOutputCount getOutputCount = NULL;

    if (term->function == NULL)
        return 1;
    FunctionAttrs* attrs = get_function_attrs(term->function);

    if (attrs == NULL)
        return 1;

    getOutputCount = attrs->getOutputCount;

    if (getOutputCount != NULL)
        return getOutputCount(term) - 1;
    
    return attrs->outputCount - 1;
}

int get_locals_count(Branch& branch)
{
    return branch.length();
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

int get_frame_distance(Branch* frame, Term* input)
{
    if (input == NULL)
        return -1;

    Branch* inputFrame = input->owningBranch;

    // If the input's branch doesn't create a separate stack frame, then look
    // at the parent branch.
    if (!branch_creates_separate_stack_frame(inputFrame))
        inputFrame = get_parent_branch(inputFrame);

    // Walk upward from 'term' until we find the common branch.
    int distance = 0;
    while (frame != inputFrame) {

        if (branch_creates_separate_stack_frame(frame))
            distance++;

        frame = get_parent_branch(frame);

        if (frame == NULL)
            return -1;
    }
    return distance;
}
int get_frame_distance(Term* term, Term* input)
{
    return get_frame_distance(term->owningBranch, input);
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
        list.inputs[i].index = input->index;
        //ca_assert(list.inputs[i].relativeFrame >= 0);
        
        // Fun special case for for-loop locals
        if (input->function == JOIN_FUNC && get_parent_term(input)->name == "#inner_rebinds")
            list.inputs[i].index = 1 + input->index;
    }
}

void update_input_instructions(Branch* branch)
{
    for (int i=0; i < branch->length(); i++)
        update_input_instructions(branch->get(i));
}

} // namespace circa
