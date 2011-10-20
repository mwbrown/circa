// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// bytecode.h
//
// Functions for creating and manipulating Circa bytecode
//

#pragma once

namespace circa {

typedef char OpType;

const OpType OP_CALL = 1;

const OpType OP_JUMP = 10;
const OpType OP_JUMP_IF = 11;
const OpType OP_JUMP_IF_NOT = 12;
const OpType OP_JUMP_IF_NOT_EQUAL = 13;
const OpType OP_JUMP_IF_LESS_THAN = 14;

const OpType OP_STOP = 20;
const OpType OP_PAUSE = 21;
const OpType OP_PAUSE_IF_ERROR = 22;

const OpType OP_PUSH_FRAME = 25;
const OpType OP_POP_FRAME = 26;

const OpType OP_INPUT_LOCAL = 31;
const OpType OP_INPUT_GLOBAL = 32;
const OpType OP_INPUT_NULL = 33;
const OpType OP_INPUT_INT = 34;
const OpType OP_OUTPUT_LOCAL = 36;

const OpType OP_ASSIGN_LOCAL = 42;

struct Operation {
    OpType type;

    void* padding1;
};

struct OpCall {
    OpType type;
    Term* func;
};

struct OpCheckOutput {
    OpType type;
    Term* term;
};

struct OpPushBranch {
    OpType type;
    Term* term;
};

struct OpLocal {
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

struct BytecodeData
{
    bool dirty;
    int operationCount;
    int localsCount;

    int stateLocal;

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

bool is_arg_op_type(OpType type);

// Printing to string
void print_bytecode_op(BytecodeData* bytecode, int loc, std::ostream& out);
void print_bytecode(BytecodeData* bytecode, std::ostream& out);
void print_bytecode_and_related(BytecodeData* bytecode, std::ostream& out);
std::string get_bytecode_as_string(BytecodeData* bytecode);

// Building functions
void bc_reserve_size(BytecodeWriter* writer, int opCount);
void bc_start_branch(BytecodeWriter* writer, Branch* branch);
int bc_get_write_position(BytecodeWriter* writer);
int bc_reserve_local(BytecodeWriter* writer);

void bc_stop(BytecodeWriter* writer);
void bc_pause(BytecodeWriter* writer);

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
void bc_local_output(BytecodeWriter* writer, int index);
void bc_write_output(BytecodeWriter* writer, Term* term);
void bc_write_input(BytecodeWriter* writer, Branch* frame, Term* input);

void bc_assign_local(BytecodeWriter* writer, int local);

// Write a copy() call. Args: (destination, source)
void bc_copy(BytecodeWriter* writer);

// Write a PUSH_FRAME operation.
void bc_push_frame(BytecodeWriter* writer, Term* term);

// Write a POP_FRAME operation.
void bc_pop_frame(BytecodeWriter* writer);

// Mark the term's owning branch as needing to recompute bytecode.
void dirty_bytecode(Term* term);
void dirty_bytecode(Branch* branch);

// Write bytecode to call the given term. This will use any override behavior,
// like the function's custom writeBytecode handler.
void bc_call(BytecodeWriter* writer, Term* term);

void bc_write_call_op(BytecodeWriter* writer, Term* function);

void bc_reset_writer(BytecodeWriter* writer);

// Refresh the branch's bytecode, if it's dirty.
void update_bytecode_for_branch(Branch* branch);
void write_bytecode_for_branch(Branch* branch, BytecodeWriter* writer);

void evaluate_bytecode(EvalContext* context, BytecodeData* bytecode);
void evaluate_branch_with_bytecode(EvalContext* context, Branch* branch);

// This can be used in Function.writeBytecode, when the call should not write
// any bytecode.
void null_bytecode_writer(BytecodeWriter*,Term*);

} // namespace circa
