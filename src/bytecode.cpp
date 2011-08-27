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

int bytecode_call(BytecodeWriter* writer, Term* term) { return 0; }
int bytecode_push_stack(BytecodeWriter* writer, int size) { return 0; }
int bytecode_pop_stack(BytecodeWriter* writer) { return 0; }
int bytecode_equals(BytecodeWriter* writer, Term* left, Term* right, int outputSlot) { return 0; }
int bytecode_jump(BytecodeWriter* writer, int offset) { return 0; }
int bytecode_jump_if_not(BytecodeWriter* writer, int inputSlot, int offset) { return 0; }
int bytecode_branch(BytecodeWriter* writer, Term* term) { return 0; }


} // namespace circa
