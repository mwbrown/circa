// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// bytecode.h
//
// Functions for creating and manipulating Circa bytecode
//
// Note: This code is still a work in progress, and isn't actually used yet.

// Operations
//
// JUMP <addr>
//   Unconditional jump to <addr>
// JUMPIF <input> <addr>
//   Jump to <addr> if <input> is true
// JUMPIFN <input> <addr>
//   Jump to <addr> if <input> is not true
// CALL
//   Execute the given C function
// BRANCH
//   Execute the given branch
// PUSH_FRAME <size>
//   Append a stack frame with the given size
// POP_FRAME
//   Discard the top stack frame
//   

#pragma once

namespace circa {

typedef char OpType;

const OpType OP_EVALUATE_TERM = 1;
const OpType OP_JUMP = 10;
const OpType OP_JUMPIF = 11;
const OpType OP_JUMPIFN = 12;
const OpType OP_CALL = 20;
const OpType OP_BRANCH = 21;


struct BytecodeWriter
{
    int writePosition;
    size_t dataSize;
    BytecodeData* data;

    BytecodeWriter() : writePosition(NULL), dataSize(0), data(NULL) {}
};

struct Operation {
    OpType type;
};

struct OpCall : Operation { Term* term; EvaluateFunc func; };

union AnyOperation {
    OpCall evaluateTerm;
};

struct BytecodeData
{
    int operationCount;
    AnyOperation operations[0];
    // 'operations' has a length of 'operationCount'.
};

void print_bytecode(Operation* op, std::ostream& out);

// Building functions
int bytecode_call(BytecodeWriter* writer, Term* term, EvaluateFunc func);

void update_bytecode_for_branch(Branch* branch);

} // namespace circa
