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

const int OP_JUMP = 1;
const int OP_JUMPIF = 2;
const int OP_JUMPIFN = 3;
const int OP_CALL = 4;
const int OP_BRANCH = 5;


struct BytecodeWriter
{
};

struct Operation {
    char type;
};

struct JumpOp : Operation { int offset; };
struct JumpIfOp : Operation { int inputSlot; int offset; };
struct JumpIfNotOp : Operation { int inputSlot; int offset; };
struct CallOp : Operation { EvaluateFunc func; Term* term; };
struct BranchOp : Operation { Term* branchTerm; };
struct PushStackOp : Operation { int size; };
struct EqualsOp : Operation { Term* left; Term* right; int outputSlot; };

union SomeOperation {
    JumpOp jumpOp;
    JumpIfOp jumpIfOp;
    JumpIfNotOp jumpIfNotOp;
    CallOp callOp;
    BranchOp branchOp;
    PushStackOp pushStackOp;
    EqualsOp equalsOp;
};

struct BytecodeData
{
    int operationCount;
    SomeOperation* operations;
};

void print_bytecode(Operation* op, std::ostream& out);

// Building functions
int bytecode_call(BytecodeWriter* writer, Term* term);
int bytecode_push_stack(BytecodeWriter* writer, int size);
int bytecode_pop_stack(BytecodeWriter* writer);
int bytecode_equals(BytecodeWriter* writer, Term* left, Term* right, int outputSlot);
int bytecode_jump(BytecodeWriter* writer, int offset);
int bytecode_jump_if_not(BytecodeWriter* writer, int inputSlot, int offset);
int bytecode_branch(BytecodeWriter* writer, Term* term);

} // namespace circa
