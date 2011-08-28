// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "bytecode.h"
#include "introspection.h"
#include "term.h"

namespace circa {

const size_t NEW_BYTECODE_DEFAULT_SIZE = 100;

void print_bytecode(Operation* op, std::ostream& out)
{
    switch (op->type) {
        case OP_CALL: {
            OpCall* cop = (OpCall*) op;
            out << "call " << global_id(cop->term);
            break;
        }
#if 0
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
#endif
    }
}

static void bytecode_reserve_size(BytecodeWriter* writer, size_t size)
{
    if (writer->data == NULL) {
        size_t allocSize = std::max(size, NEW_BYTECODE_DEFAULT_SIZE);
        writer->data = (BytecodeData*) malloc(sizeof(BytecodeData) + allocSize);
        writer->writePosition = 0;
        writer->dataSize = allocSize;
        return;
    }

    if (writer->dataSize < size) {
        writer->dataSize = size;
        writer->data = (BytecodeData*) realloc(writer->data, writer->dataSize);
    }
}

Operation* bytecode_append_op(BytecodeWriter* writer, OpType type)
{
    size_t size = sizeof(AnyOperation);
    bytecode_reserve_size(writer, size);
    Operation* result = (Operation*) &writer->data->operations[writer->writePosition];
    writer->writePosition += size;
    return result;
}

int bytecode_call(BytecodeWriter* writer, Term* term)
{
    return 0;
}

void update_bytecode_for_branch(Branch* branch)
{
    BytecodeWriter writer;
}

} // namespace circa
