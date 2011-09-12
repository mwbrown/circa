// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "builtins.h"
#include "bytecode.h"
#include "if_block.h"
#include "importing_macros.h"
#include "list_shared.h"
#include "term.h"
#include "type.h"

namespace circa {

void switch_block_post_compile(Term* term)
{
    // Add a default case
    apply(nested_contents(term), DEFAULT_CASE_FUNC, TermList());
    update_if_block_joining_branch(term);
}

void switch_block_write_bytecode(Term* caller, BytecodeWriter* writer)
{
#if 0
    Branch& contents = nested_contents(term);
    int numCases = contents.length() - 1;

    // Push a stack frame, we'll use this when evaluating term equality.
    bc_push_stack(writer, numCases);

    Term* inputTerm = term->input(0);

    std::vector<int> jumpsToFinish;

    // Switch bytecode looks like:
    //   # first case:
    //   equals <switchInput> <caseInput>
    //   jump_if_not <equals result> <to next case>
    //   # case was successful
    //   branch <case term>
    //   jump <to 'finish'>
    //   <.. repeated for each case ..>
    //   # finish
    //   ...

    for (int caseIndex=0; caseIndex < numCases; caseIndex++) {
        Term* caseTerm = contents[caseIndex];
        int outputSlot = caseIndex;
        bc_equals(writer, inputTerm, caseTerm->input(0), outputSlot);
        bc_jump_if_not(writer, outputSlot, 2);
        bc_branch(writer, caseTerm);
        int finishJump = bc_jump(writer, 0);
        jumpsToFinish.push_back(finishJump);
    }
#endif

    // start_switch <input count>
    //  <input list>
    // call_branch
    // copy output from stack
    // pop_stack

    // jump_if_not next_case
    //  input0
    // do stuff for this case
    // next_case: jump_if_not next_case_2

    Branch& contents = nested_contents(caller);

    for (int caseIndex=0; caseIndex < contents.length()-1; caseIndex++) {
        Term* caseTerm = contents[caseIndex];

        //Operation* initial_jump_if_not = bc_jump_if_not(writer);
        //bc_write_input(writer, caseTerm, &contents);

        //bc_call_branch(writer, caseTerm);

        //bc_jump_to_here(writer, initial_jump_if_not);
    }
}

CA_FUNCTION(evaluate_switch)
{
    EvalContext* context = CONTEXT;
    Term* caller = CALLER;
    Branch& contents = nested_contents(CALLER);
    TaggedValue* input = INPUT(0);

    // Iterate through each 'case' and find one that succeeds.
    for (int caseIndex=0; caseIndex < contents.length()-1; caseIndex++) {
        Term* caseTerm = contents[caseIndex];

        bool succeeds = false;
        if (caseTerm->function == DEFAULT_CASE_FUNC) {
            succeeds = true;
        } else if (caseTerm->function == CASE_FUNC) {
            TaggedValue* caseValue = get_input(context, caseTerm, 0);
            succeeds = equals(input, caseValue);
        } else {
            internal_error("unrecognized function inside switch()");
        }

        if (succeeds) {
            Branch& caseContents = nested_contents(caseTerm);
            push_stack_frame(context, &caseContents);

            // Evaluate contents
            evaluate_branch_with_bytecode(context, &caseContents);

            // Copy joined values to output slots
            Branch& joining = nested_contents(contents.getFromEnd(0));

            for (int i=0; i < joining.length(); i++) {
                Term* joinTerm = joining[i];
                TaggedValue* value = get_input(context, joinTerm, caseIndex);

                ca_test_assert(cast_possible(value, get_output_type(CALLER, i+1)));

                int outputIndex = caller->index + 1 + i;
                TaggedValue* dest = list_get_index(get_stack_frame(context, 1), outputIndex);
                copy(value, dest);
            }

            pop_stack_frame(context);
            break;
        }
    }
}

} // namespace circa
