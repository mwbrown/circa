// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// bytecode.h
//
// Functions for creating and manipulating Circa bytecode
//
// Note: This code is still a work in progress, and isn't actually used yet.

// Operations
//

#pragma once

namespace circa {

const OpType OP_CALL = 20;
const OpType OP_RETURN = 21;
const OpType OP_RETURN_ON_ERROR = 22;
const OpType OP_STACK_SIZE = 23;
const OpType OP_JUMP_IF = 27;
const OpType OP_JUMP_IF_NOT = 27;

const OpType OP_INPUT_LOCAL = 24;
const OpType OP_INPUT_GLOBAL = 25;
const OpType OP_INPUT_NULL = 26;

struct Operation {
    OpType type;

    // padding..
    void* ptr1;
    void* ptr2;
};

// OpCall is defined in common_headers.h

struct OpStackSize {
    OpType type;
    int size;
};

struct OpInputLocal {
    OpType type;
    int relativeFrame;
    int index;
};

struct OpInputGlobal {
    OpType type;
    TaggedValue* value;
};

struct OpJumpIf {
    OpType type;
    int offset;
};

struct BytecodeData
{
    bool dirty;
    int operationCount;
    Operation operations[0];
    // 'operations' has a length of at least 'operationCount'.
};

struct BytecodeWriter
{
    int writePosition;
    int listLength;
    BytecodeData* data;

    // If an InputOverride function is installed, it will be called whenever
    // writing an input instruction. The function can change whether the result
    // is a local or global.
    typedef void (*InputOverride)(void* cxt, Term* term, Operation* op);
    InputOverride inputOverride;
    void* inputOverrideContext;

    BytecodeWriter()
      : writePosition(0),
        listLength(0),
        data(NULL),
        inputOverride(NULL),
        inputOverrideContext(NULL)
    {}
    ~BytecodeWriter() { free(data); }
};

void print_bytecode(BytecodeData* bytecode, std::ostream& out);
std::string get_bytecode_as_string(BytecodeData* bytecode);

// Building functions
void bc_write_call_op(BytecodeWriter* writer, Term* term, EvaluateFunc func);
void bc_return(BytecodeWriter* writer);
OpJumpIf* bc_jump_if(BytecodeWriter* writer);
OpJumpIf* bc_jump_if_not(BytecodeWriter* writer);
void bc_write_input(BytecodeWriter* writer, Branch* frame, Term* input);
void bc_write_global_input(Operation* op, TaggedValue* value);
void bc_write_local_input(Operation* op, int frame, int index);

// Mark the term's owning branch as needing to recompute bytecode.
void dirty_bytecode(Term* term);
void dirty_bytecode(Branch& branch);

// Write bytecode to call the given term. This will use any custom behavior,
// like the function's custom writeBytecode handler.
void bc_call(BytecodeWriter* writer, Term* term);

void bc_finish(BytecodeWriter* writer);

// Refresh the branch's bytecode, if it's dirty.
void update_bytecode_for_branch(Branch* branch);

void evaluate_bytecode(EvalContext* context, BytecodeData* bytecode);
void evaluate_branch_with_bytecode(EvalContext* context, Branch* branch);

// This can be used in Function.writeBytecode, when the call should not write
// any bytecode.
void null_bytecode_writer(Term*, BytecodeWriter*);

} // namespace circa
