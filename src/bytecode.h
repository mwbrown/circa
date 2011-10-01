// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// bytecode.h
//
// Functions for creating and manipulating Circa bytecode
//

#pragma once

namespace circa {

typedef char OpType;

const OpType OP_CALL = 1;
const OpType OP_CALL_BRANCH = 2;
const OpType OP_CHECK_OUTPUT = 3;

const OpType OP_JUMP = 10;
const OpType OP_JUMP_IF = 11;
const OpType OP_JUMP_IF_NOT = 12;
const OpType OP_JUMP_IF_NOT_EQUAL = 13;
const OpType OP_JUMP_IF_WITHIN_RANGE = 14;

const OpType OP_RETURN = 20;
const OpType OP_RETURN_ON_ERROR = 21;
const OpType OP_POP_STACK = 23;

const OpType OP_INPUT_LOCAL = 31;
const OpType OP_INPUT_GLOBAL = 32;
const OpType OP_INPUT_NULL = 33;
const OpType OP_INPUT_INT = 34;

const OpType OP_COPY = 40;
const OpType OP_INCREMENT = 41;

struct Operation {
    OpType type;

    // padding..
    void* ptr1;
    void* ptr2;
    int padding3;
};

struct OpCall {
    OpType type;
    Term* term;
    EvaluateFunc func;
};

struct OpCheckOutput {
    OpType type;
    Term* term;
};

struct OpCallBranch {
    OpType type;
    Term* term;
};
struct OpStackSize {
    OpType type;
    int size;
};

struct OpInputLocal {
    OpType type;
    short relativeFrame;
    short index;
};

struct OpInputGlobal {
    OpType type;
    TaggedValue* value;
};

struct OpInputInt {
    OpType type;
    int value;
};

struct OpJump {
    OpType type;
    int offset;
};

struct BytecodeData
{
    bool dirty;
    int operationCount;
    int stackSize;
    Operation operations[0];
    // 'operations' has a length of at least 'operationCount'.
};

struct BytecodeWriter
{
    int writePosition;
    int listLength;
    BytecodeData* data;

    // Configuration options
    bool alwaysCheckOutputs;

    BytecodeWriter()
      : writePosition(0),
        listLength(0),
        data(NULL),
        alwaysCheckOutputs(false)
    {}
    ~BytecodeWriter() { free(data); }
};

void print_bytecode(BytecodeData* bytecode, std::ostream& out);
std::string get_bytecode_as_string(BytecodeData* bytecode);

// Building functions
void bc_write_call_op(BytecodeWriter* writer, Term* term, EvaluateFunc func);
void bc_write_call_op_with_func(BytecodeWriter* writer, Term* term, Term* func);
void bc_check_output(BytecodeWriter* writer, Term* term);
void bc_return(BytecodeWriter* writer);

// Write a CALL instruction with no Term*, just an EvaluateFunc. Input
// instructions must be appended by the caller. Some functions don't work
// with a NULL caller.
void bc_imaginary_call(BytecodeWriter* writer, EvaluateFunc func, int output);
void bc_imaginary_call(BytecodeWriter* writer, Term* func, int output);

// Write a JUMP instruction. The jump is initialized without an offset,
// caller should use bc_jump_to_here() to set one.
int bc_jump(BytecodeWriter* writer);

// Write a JUMP_IF instruction. The jump is initialized without an offset,
// caller should use bc_jump_to_here() to set one. One input instruction must
// follow.
int bc_jump_if(BytecodeWriter* writer);

// Write a JUMP_IF_NOT operation. See notes for bc_jump_if().
int bc_jump_if_not(BytecodeWriter* writer);

// Write a JUMP_IF_NOT_EQUAL operation. The jump is initialized without an offset,
// caller should use bc_jump_to_here() to set one. One input instruction must
// follow.
int bc_jump_if_not_equal(BytecodeWriter* writer);

// Write a JUMP_IF_WITHIN_RANGE operation. The jump is initialized without an offset,
// caller should use bc_jump_to_here() to set one. Two input instructions must
// follow, input 0 is a list and input 1 is an index. The jump will be performed if
// the index is less than the length of the list.
int bc_jump_if_within_range(BytecodeWriter* writer);

// Modify the jump operation to jump to the current write position.
void bc_jump_to_here(BytecodeWriter* writer, int jumpLoc);

// Modify the jump operation to jump to the given position.
void bc_jump_to_pos(BytecodeWriter* writer, int jumpLoc, int pos);

void bc_global_input(BytecodeWriter* writer, TaggedValue* value);
void bc_local_input(BytecodeWriter* writer, int frame, int index);

void bc_write_input(BytecodeWriter* writer, Branch* frame, Term* input);
void bc_write_int_input(BytecodeWriter* writer, int value);

// Write a COPY operation. Two input instructions must follow.
void bc_copy_value(BytecodeWriter* writer);

// Write an INCREMENT operation. One input instruction must follow.
void bc_increment(BytecodeWriter* writer);

// Write a CALL_BRANCH operation.
void bc_call_branch(BytecodeWriter* writer, Term* term);

// Write a POP_STACK operation.
void bc_pop_stack(BytecodeWriter* writer);

// Mark the term's owning branch as needing to recompute bytecode.
void dirty_bytecode(Term* term);
void dirty_bytecode(Branch* branch);

// Write bytecode to call the given term. This will use any custom behavior,
// like the function's custom writeBytecode handler.
void bc_call(BytecodeWriter* writer, Term* term);

void bc_finish(BytecodeWriter* writer);

void bc_reset_writer(BytecodeWriter* writer);

// Refresh the branch's bytecode, if it's dirty.
void update_bytecode_for_branch(Branch* branch);
void write_bytecode_for_branch(Branch* branch, BytecodeWriter* writer);

void evaluate_bytecode(EvalContext* context, BytecodeData* bytecode);
void evaluate_branch_with_bytecode(EvalContext* context, Branch* branch);

// This can be used in Function.writeBytecode, when the call should not write
// any bytecode.
void null_bytecode_writer(Term*, BytecodeWriter*);

} // namespace circa
