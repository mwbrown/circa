// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "bytecode.h"
#include "introspection.h"
#include "term.h"

namespace circa {

void print_bytecode(Operation* op, std::ostream& out)
{
#if 0
    switch (op->type) {
        case OP_JUMP: {
            JumpOp* jop = (JumpOp*) op;
            out << "jump " << jop->offset;
            break;
        }
        case OP_JUMPIF: {
            JumpIfOp* jop = (JumpIfOp*) op;
            out << "jumpif " << global_id(jop->input) << " " << jop->offset;
            break;
        }
        case OP_JUMPIFN: {
            JumpIfOp* jop = (JumpIfOp*) op;
            out << "jumpif " << global_id(jop->input) << " " << jop->offset;
            break;
        }
        case OP_CALL: {
            JumpIfNotOp* jop = (JumpIfNotOp*) op;
            out << "jumpifn " << global_id(jop->input) << " " << jop->offset;
            break;
        }
        case OP_BRANCH: {
            BranchOp* bop = (BranchOp*) op;
            out << "branch " << global_id(bop->branchTerm);
            break;
        }
    }
#endif
}

void bytecode_write_call(BytecodeWriter* writer, Term* term)
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

} // namespace circa
