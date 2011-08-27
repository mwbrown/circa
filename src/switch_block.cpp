// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "builtins.h"
#include "bytecode.h"
#include "if_block.h"
#include "importing_macros.h"
#include "term.h"
#include "type.h"

namespace circa {

void switch_block_post_compile(Term* term)
{
    // Add a default case
    apply(nested_contents(term), DEFAULT_CASE_FUNC, TermList());
    update_if_block_joining_branch(term);
}

void switch_block_write_bytecode(Term* term, BytecodeWriter* writer)
{
    Branch& contents = nested_contents(term);
    int numCases = contents.length() - 1;

    // Push a stack frame, we'll use this when evaluating term equality.
    bytecode_push_stack(writer, numCases);

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
        bytecode_equals(writer, inputTerm, caseTerm->input(0), outputSlot);
        bytecode_jump_if_not(writer, outputSlot, 2);
        bytecode_branch(writer, caseTerm);
        int finishJump = bytecode_jump(writer, 0);
        jumpsToFinish.push_back(finishJump);
    }
}



CA_FUNCTION(evaluate_switch)
{
    EvalContext* context = CONTEXT;
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
            start_using(caseContents);

            for (int i=0; i < caseContents.length(); i++) {
                evaluate_single_term(context, caseContents[i]);

                  if (evaluation_interrupted(context))
                      break;
            }

            // Copy joined values to output slots
            Branch& joining = nested_contents(contents.getFromEnd(0));

            for (int i=0; i < joining.length(); i++) {
                Term* joinTerm = joining[i];
                TaggedValue* value = get_input(context, joinTerm, caseIndex);

                ca_test_assert(cast_possible(value, get_output_type(CALLER, i+1)));

                copy(value, EXTRA_OUTPUT(i));
            }

            finish_using(caseContents);
            break;
        }
    }
}

} // namespace circa
