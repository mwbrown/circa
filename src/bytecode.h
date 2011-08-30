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

const OpType OP_CALL = 20;
const OpType OP_RETURN = 21;
const OpType OP_RETURN_ON_ERROR = 22;
const OpType OP_RETURN_IF_INTERRUPTED = 23; // <- for backwards compatibility

// future:
const OpType OP_EVALUATE_TERM = 1;
const OpType OP_JUMP = 10;
const OpType OP_JUMPIF = 11;
const OpType OP_JUMPIFN = 12;
const OpType OP_BRANCH = 21;


struct BytecodeWriter
{
    int writePosition;
    int listLength;
    BytecodeData* data;

    BytecodeWriter() : writePosition(NULL), listLength(0), data(NULL) {}
    ~BytecodeWriter() { free(data); }
};

struct Operation {
    OpType type;

    // padding..
    void* ptr1;
    void* ptr2;
};

struct OpCall {
    OpType type;
    Term* term;
    EvaluateFunc func;
};

struct BytecodeData
{
    bool dirty;
    int operationCount;
    Operation operations[0];
    // 'operations' has a length of at least 'operationCount'.
};

void print_bytecode(Operation* op, std::ostream& out);

// Building functions
int bytecode_call(BytecodeWriter* writer, Term* term, EvaluateFunc func);
int bytecode_return(BytecodeWriter* writer, Term* term, EvaluateFunc func);

void update_bytecode_for_branch(Branch* branch);
void evaluate_branch_with_bytecode(EvalContext* context, Branch* branch);

} // namespace circa
