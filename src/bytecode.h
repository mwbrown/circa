// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// bytecode.h
//
// Functions for creating and manipulating Circa bytecode
//

#pragma once

namespace circa {

typedef char OpType;

const OpType OP_CALL = 1;
const OpType OP_CHECK_OUTPUT = 3;

const OpType OP_JUMP = 10;
const OpType OP_JUMP_IF = 11;
const OpType OP_JUMP_IF_NOT = 12;
const OpType OP_JUMP_IF_NOT_EQUAL = 13;
const OpType OP_JUMP_IF_LESS_THAN = 14;

const OpType OP_STOP = 20;

const OpType OP_PUSH_FRAME = 22;
const OpType OP_POP_FRAME = 23;

const OpType OP_INPUT_LOCAL = 31;
const OpType OP_INPUT_GLOBAL = 32;
const OpType OP_INPUT_NULL = 33;
const OpType OP_INPUT_INT = 34;

const OpType OP_ASSIGN_LOCAL = 42;
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

struct OpPushBranch {
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
    short local;
};

struct OpInputGlobal {
    OpType type;
    TaggedValue* value;
};

struct OpInputInt {
    OpType type;
    int value;
};

struct OpAssignLocal {
    OpType type;
    int local;
};

struct OpJump {
    OpType type;
    int offset;
};

struct BytecodeGenerationFlags
{
    bool alwaysCheckOutputs : 1;
    bool useGlobals : 1;
};

struct BytecodeData
{
    bool dirty;
    int operationCount;
    int localsCount;
    BytecodeGenerationFlags flags;

    Branch* branch;
    Operation operations[0];
    // 'operations' has a length of at least 'operationCount'.
};

struct BytecodeWriter
{
    int writePosition;
    int listLength;
    BytecodeData* data;

    BytecodeWriter()
      : writePosition(0),
        listLength(0),
        data(NULL)
    {}
    ~BytecodeWriter() { free(data); }
};

// Printing to string
void print_bytecode_op(BytecodeData* bytecode, int loc, std::ostream& out);
void print_bytecode(BytecodeData* bytecode, std::ostream& out);
void print_bytecode_and_related(BytecodeData* bytecode, std::ostream& out);
std::string get_bytecode_as_string(BytecodeData* bytecode);

// Building functions
void bc_reserve_size(BytecodeWriter* writer, int opCount);
int bc_get_write_position(BytecodeWriter* writer);

void bc_write_call_op(BytecodeWriter* writer, Term* term, EvaluateFunc func);
void bc_write_call_op_with_func(BytecodeWriter* writer, Term* term, Term* func);
void bc_write_call(BytecodeWriter* writer, Term* function);

void bc_check_output(BytecodeWriter* writer, Term* term);
void bc_stop(BytecodeWriter* writer);

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

// Write a JUMP_IF_NOT_EQUAL operation. The jump is initialized without an offset,
// caller should use bc_jump_to_here() to set one. Two input instructions must
// follow.
int bc_jump_if_less_than(BytecodeWriter* writer);

// Modify the jump operation to jump to the current write position.
void bc_jump_to_here(BytecodeWriter* writer, int jumpLoc);

// Modify the jump operation to jump to the given position.
void bc_jump_to_pos(BytecodeWriter* writer, int jumpLoc, int pos);

void bc_global_input(BytecodeWriter* writer, TaggedValue* value);
void bc_local_input(BytecodeWriter* writer, int index);
void bc_local_input(BytecodeWriter* writer, Term* term);
void bc_local_input(BytecodeWriter* writer, int frame, int index);
void bc_int_input(BytecodeWriter* writer, int value);

void bc_write_input(BytecodeWriter* writer, Branch* frame, Term* input);

void bc_assign_local(BytecodeWriter* writer, int local);

// Write a copy() call. Args: (destination, source)
void bc_copy(BytecodeWriter* writer);

// Write an INCREMENT operation. One input instruction must follow.
void bc_increment(BytecodeWriter* writer);

// Write a PUSH_FRAME operation.
void bc_push_frame(BytecodeWriter* writer, Term* term);

// Write a POP_FRAME operation.
void bc_pop_frame(BytecodeWriter* writer);

// Mark the term's owning branch as needing to recompute bytecode.
void dirty_bytecode(Term* term);
void dirty_bytecode(Branch* branch);

// Write bytecode to call the given term. This will use any custom behavior,
// like the function's custom writeBytecode handler.
void bc_call(BytecodeWriter* writer, Term* term);

void bc_reset_writer(BytecodeWriter* writer);

// Set flags
void bc_always_check_outputs(BytecodeWriter* writer);

// Refresh the branch's bytecode, if it's dirty.
void update_bytecode_for_branch(Branch* branch);
void write_bytecode_for_branch(Branch* branch, BytecodeWriter* writer);

void evaluate_bytecode(EvalContext* context, BytecodeData* bytecode);
void evaluate_branch_with_bytecode(EvalContext* context, Branch* branch);

// This can be used in Function.writeBytecode, when the call should not write
// any bytecode.
void null_bytecode_writer(BytecodeWriter*,Term*);

} // namespace circa
