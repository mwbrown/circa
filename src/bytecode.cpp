// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include <set>

#include "building.h"
#include "bytecode.h"
#include "evaluation.h"
#include "function.h"
#include "introspection.h"
#include "list_shared.h"
#include "kernel.h"
#include "refactoring.h"
#include "term.h"
#include "type.h"

namespace circa {

const int NEW_BYTECODE_DEFAULT_LENGTH = 6;

static bool is_input_op_type(OpType type)
{
    switch (type) {
        case OP_INPUT_LOCAL:
        case OP_INPUT_GLOBAL:
        case OP_INPUT_NULL:
        case OP_INPUT_INT:
            return true;
        default:
            return false;
    }
}

static bool is_jump_op_type(OpType type)
{
    switch (type) {
        case OP_JUMP:
        case OP_JUMP_IF:
        case OP_JUMP_IF_NOT:
        case OP_JUMP_IF_NOT_EQUAL:
        case OP_JUMP_IF_LESS_THAN:
            return true;
        default:
            return false;
    }
}

void print_bytecode_op(BytecodeData* bytecode, int loc, std::ostream& out)
{
    Operation* op = &bytecode->operations[loc];

    switch (op->type) {
        case OP_CALL: {
            out << "call ";
            Term* term = ((OpCall*) op)->term;
            if (term == NULL)
                out << "NULL";
            else
                out << get_unique_name(term);
            break;
        }
        case OP_CHECK_OUTPUT:
            out << "check_output " << get_unique_name(((OpCheckOutput*) op)->term);
            break;
        case OP_INPUT_NULL:
            out << "null_arg";
            break;
        case OP_INPUT_GLOBAL: {
            OpInputGlobal* gop = (OpInputGlobal*) op;
            out << "global_arg ";
            if (gop->value == NULL)
               out << "<NULL>";
            else
               out << gop->value->toString();
            break;
        }
        case OP_INPUT_LOCAL: {
            OpInputLocal* lop = (OpInputLocal*) op;
            out << "local_arg ";
            if (lop->relativeFrame != 0)
                out << "frame:" << lop->relativeFrame << " ";
            out << "idx:" << lop->local;
            break;
        }
        case OP_INPUT_INT: {
            OpInputInt* iop = (OpInputInt*) op;
            out << "input_int " << iop->value;
            break;
        }
        case OP_STOP:
            out << "stop";
            break;
        case OP_JUMP:
            out << "jump " << loc + ((OpJump*) op)->offset;
            break;
        case OP_JUMP_IF:
            out << "jump_if " << loc + ((OpJump*) op)->offset;
            break;
        case OP_JUMP_IF_NOT:
            out << "jump_if_not " << loc + ((OpJump*) op)->offset;
            break;
        case OP_JUMP_IF_NOT_EQUAL:
            out << "jump_if_not_eq " << loc + ((OpJump*) op)->offset;
            break;
        case OP_JUMP_IF_LESS_THAN:
            out << "jump_if_less_than " << loc + ((OpJump*) op)->offset;
            break;
        case OP_POP_FRAME:
            out << "pop_frame";
            break;
        case OP_PUSH_FRAME:
            out << "push_frame " << global_id(((OpPushBranch*) op)->term);
            break;
        case OP_INCREMENT:
            out << "increment";
            break;
        case OP_ASSIGN_LOCAL:
            out << "assign_local " << ((OpAssignLocal*) op)->local;
            break;
        default:
            out << "<unknown opcode " << int(op->type) << ">";
    }
}

void print_bytecode(BytecodeData* bytecode, std::ostream& out)
{
    Term* owner = NULL;
    if (bytecode->branch != NULL)
        owner = bytecode->branch->owningTerm;

    out << "[Bytecode ";
    if (owner != NULL)
        out << global_id(owner);
    else
        out << "<orphan @" << bytecode << ">";

    out << ", localsCount = " << bytecode->localsCount << "]" << std::endl;

    for (int i=0; i < bytecode->operationCount; i++) {

        bool isInput = is_input_op_type(bytecode->operations[i].type);

        if (i != 0) {
            if (isInput)
                out << "; ";
            else
                out << "; " << std::endl;
        }

        if (!isInput)
            std::cout << i << ": ";

        print_bytecode_op(bytecode, i, out);
    }
    out << std::endl;
}

void print_bytecode_and_related(BytecodeData* bytecode, std::ostream& out)
{
    print_bytecode(bytecode, out);

    // Also print out any branches that were mentioned
    std::set<Term*> mentionedBranches;

    for (int i=0; i < bytecode->operationCount; i++) {
        if (bytecode->operations[i].type == OP_PUSH_FRAME) {
            OpPushBranch* op = (OpPushBranch*) &bytecode->operations[i];
            mentionedBranches.insert(op->term);
        }
    }

    std::set<Term*>::iterator it;
    for (it = mentionedBranches.begin(); it != mentionedBranches.end(); ++it) {

        // Evil step: update bytecode before printing. We shouldn't have side
        // effects here.
        update_bytecode_for_branch(nested_contents(*it));
        
        BytecodeData* mentionedBytecode = nested_contents(*it)->bytecode;
        if (mentionedBytecode != NULL)
            print_bytecode(mentionedBytecode, out);
        else
            std::cout << "[NULL bytecode " << global_id(*it) << "]" << std::endl;
    }
}

std::string get_bytecode_as_string(BytecodeData* bytecode);

static void start_bytecode_update(Branch* branch, BytecodeWriter* writer)
{
    // Move BytecodeData from the Branch to the BytecodeWriter. We will give it
    // back soon.
    writer->data = branch->bytecode;
    writer->writePosition = 0;
    branch->bytecode = NULL;
}

static void finish_bytecode_update(Branch* branch, BytecodeWriter* writer)
{
    ca_assert(branch->bytecode == NULL);
    branch->bytecode = writer->data;
    writer->data = NULL;
}

// Guarantee that the BytecodeData has enough room for the given number
// of operations.
void bc_reserve_size(BytecodeWriter* writer, int opCount)
{
    if (writer->listLength >= opCount && writer->data != NULL)
        return;

    int newLength = opCount;
    
    if (writer->data == NULL) {
        newLength = std::max(newLength, NEW_BYTECODE_DEFAULT_LENGTH);
        writer->data = (BytecodeData*) malloc(
                sizeof(BytecodeData) + sizeof(Operation) * newLength);
        writer->data->operationCount = 0;
        writer->data->localsCount = 0;
        writer->data->dirty = false;
        writer->data->localsCount = 0;
        memset(&writer->data->flags, 0, sizeof(writer->data->flags));
        writer->data->branch = NULL;
    } else {
        writer->data = (BytecodeData*) realloc(writer->data,
                sizeof(BytecodeData) + sizeof(Operation) * newLength);
    }

    writer->listLength = newLength;
}
int bc_get_write_position(BytecodeWriter* writer)
{
    return writer->writePosition;
}

// Appends a slot for an operation, returns the operation's index.
Operation* bc_append_op(BytecodeWriter* writer)
{
    int pos = writer->writePosition++;
    bc_reserve_size(writer, writer->writePosition);
    writer->data->operationCount++;
    return &writer->data->operations[pos];
}

void bc_write_call_op(BytecodeWriter* writer, Term* term, EvaluateFunc func)
{
    OpCall* op = (OpCall*) bc_append_op(writer);
    op->type = OP_CALL;
    op->term = term;
    op->func = func;

    // Write output instruction.
    bc_write_input(writer, term->owningBranch, term);

    // Write information for each input
    for (int i=0; i < term->numInputs(); i++)
        bc_write_input(writer, term->owningBranch, term->input(i));

    // Possibly write a CHECK_OUTPUT op.
    if (writer->data->flags.alwaysCheckOutputs || DEBUG_ALWAYS_CHECK_OUTPUT_TYPE)
        bc_check_output(writer, term);
}

void bc_write_call_op_with_func(BytecodeWriter* writer, Term* term, Term* func)
{
    bc_write_call_op(writer, term, get_function_attrs(func)->evaluate);
}
void bc_write_call(BytecodeWriter* writer, Term* function)
{
    OpCall* op = (OpCall*) bc_append_op(writer);
    op->type = OP_CALL;
    op->term = function;
    op->func = get_function_attrs(function)->evaluate;
}

void bc_stop(BytecodeWriter* writer)
{
    bc_append_op(writer)->type = OP_STOP;
}

void bc_imaginary_call(BytecodeWriter* writer, EvaluateFunc func, int output)
{
    OpCall* op = (OpCall*) bc_append_op(writer);
    op->type = OP_CALL;
    op->term = NULL;
    op->func = func;

    // Write output instruction
    bc_local_input(writer, 0, output);
}
void bc_imaginary_call(BytecodeWriter* writer, Term* func, int output)
{
    bc_imaginary_call(writer, get_function_attrs(func)->evaluate, output);
}
int bc_jump(BytecodeWriter* writer)
{
    int pos = writer->writePosition;
    OpJump* op = (OpJump*) bc_append_op(writer);
    op->type = OP_JUMP;
    op->offset = 0;
    return pos;
}
int bc_jump_if(BytecodeWriter* writer)
{
    int pos = writer->writePosition;
    OpJump* op = (OpJump*) bc_append_op(writer);
    op->type = OP_JUMP_IF;
    op->offset = 0;
    return pos;
}
int bc_jump_if_not(BytecodeWriter* writer)
{
    int pos = writer->writePosition;
    OpJump* op = (OpJump*) bc_append_op(writer);
    op->type = OP_JUMP_IF_NOT;
    op->offset = 0;
    return pos;
}
int bc_jump_if_not_equal(BytecodeWriter* writer)
{
    int pos = writer->writePosition;
    OpJump* op = (OpJump*) bc_append_op(writer);
    op->type = OP_JUMP_IF_NOT_EQUAL;
    op->offset = 0;
    return pos;
}
int bc_jump_if_less_than(BytecodeWriter* writer)
{
    int pos = writer->writePosition;
    OpJump* op = (OpJump*) bc_append_op(writer);
    op->type = OP_JUMP_IF_LESS_THAN;
    op->offset = 0;
    return pos;
}
void bc_jump_to_here(BytecodeWriter* writer, int jumpLoc)
{
    OpJump* op = (OpJump*) &writer->data->operations[jumpLoc];
    ca_assert(is_jump_op_type(op->type));
    op->offset = writer->writePosition - jumpLoc;
}
void bc_jump_to_pos(BytecodeWriter* writer, int jumpLoc, int pos)
{
    OpJump* op = (OpJump*) &writer->data->operations[jumpLoc];
    ca_assert(is_jump_op_type(op->type));
    op->offset = pos - jumpLoc;
}
void bc_global_input(BytecodeWriter* writer, TaggedValue* value)
{
    OpInputGlobal* gop = (OpInputGlobal*) bc_append_op(writer);
    gop->type = OP_INPUT_GLOBAL;
    gop->value = (TaggedValue*) value;
}
void bc_local_input(BytecodeWriter* writer, int local)
{
    OpInputLocal *lop = (OpInputLocal*) bc_append_op(writer);
    lop->type = OP_INPUT_LOCAL;
    lop->relativeFrame = 0;
    lop->local = local;
}
void bc_local_input(BytecodeWriter* writer, Term* term)
{
    ca_assert(term->local != -1);
    bc_local_input(writer, term->local);
}
void bc_local_input(BytecodeWriter* writer, int frame, int local)
{
    OpInputLocal *lop = (OpInputLocal*) bc_append_op(writer);
    lop->type = OP_INPUT_LOCAL;
    lop->relativeFrame = frame;
    lop->local = local;
}
void bc_write_input(BytecodeWriter* writer, Branch* frame, Term* input)
{
    if (input == NULL) {
        bc_append_op(writer)->type = OP_INPUT_NULL;
        return;
    }

    if (is_value(input) || writer->data->flags.useGlobals) {
        bc_global_input(writer, (TaggedValue*) input);
    } else {
        // Walk both 'frame' and 'input' upward, if they are in a branch that
        // does not create a stack frame.
        while (!branch_creates_stack_frame(frame))
            frame = get_parent_branch(frame);
        while (!branch_creates_stack_frame(input->owningBranch))
            input = get_parent_term(input);

        int relativeFrame = get_frame_distance(frame, input);
        bc_local_input(writer, relativeFrame, input->local);
    }
}
void bc_int_input(BytecodeWriter* writer, int value)
{
    OpInputInt *iop = (OpInputInt*) bc_append_op(writer);
    iop->type = OP_INPUT_INT;
    iop->value = value;
}
void bc_assign_local(BytecodeWriter* writer, int local)
{
    OpAssignLocal *aop = (OpAssignLocal*) bc_append_op(writer);
    aop->type = OP_ASSIGN_LOCAL;
    aop->local = local;
}
void bc_copy(BytecodeWriter* writer)
{
    bc_write_call(writer, COPY_FUNC);
}
void bc_increment(BytecodeWriter* writer)
{
    bc_append_op(writer)->type = OP_INCREMENT;
}
void bc_push_frame(BytecodeWriter* writer, Term* term)
{
    OpPushBranch *cop = (OpPushBranch*) bc_append_op(writer);
    cop->type = OP_PUSH_FRAME;
    cop->term = term;
}
void bc_pop_frame(BytecodeWriter* writer)
{
    bc_append_op(writer)->type = OP_POP_FRAME;
}

void dirty_bytecode(Term* term)
{
    if (term->owningBranch != NULL)
        dirty_bytecode(term->owningBranch);
}

void dirty_bytecode(Branch* branch)
{
    if (branch->bytecode != NULL)
        branch->bytecode->dirty = true;
}

void bc_call(BytecodeWriter* writer, Term* term)
{
    // NULL function: no bytecode
    if (term->function == NULL)
        return;

    // Function isn't a function: no bytecode
    if (!is_function(term->function))
        return;

    if (term->function == UNKNOWN_IDENTIFIER_FUNC)
        return;

    // Don't write anything for certain special names
    if (term->name == "#inner_rebinds"
            || term->name == "#outer_rebinds")
        return;

    // Check if the function has a special writer function
    FunctionAttrs::WriteBytecode writeBytecode =
        get_function_attrs(term->function)->writeBytecode;

    if (writeBytecode != NULL)
        return writeBytecode(writer, term);

    EvaluateFunc evaluateFunc = get_function_attrs(term->function)->evaluate;

    // NULL evaluateFunc: no bytecode
    if (evaluateFunc == NULL)
        return;

    // Default: Add an OP_CALL
    bc_write_call_op(writer, term, evaluateFunc);
}

void bc_check_output(BytecodeWriter* writer, Term* term)
{
    OpCheckOutput* op = (OpCheckOutput*) bc_append_op(writer);
    op->type = OP_CHECK_OUTPUT;
    op->term = term;
}

void bc_reset_writer(BytecodeWriter* writer)
{
    writer->writePosition = 0;
    if (writer->data != NULL)
        writer->data->operationCount = 0;
}
void bc_always_check_outputs(BytecodeWriter* writer)
{
    bc_reserve_size(writer, 0);
    writer->data->flags.alwaysCheckOutputs = true;
}

void update_bytecode_for_branch(Branch* branch)
{
    // Don't update if bytecode is not dirty.
    if (branch->bytecode != NULL && !branch->bytecode->dirty)
        return;

    BytecodeWriter writer;
    start_bytecode_update(branch, &writer);

    write_bytecode_for_branch(branch, &writer);

    finish_bytecode_update(branch, &writer);
}

void write_bytecode_for_branch(Branch* branch, BytecodeWriter* writer)
{
    Term* parent = branch->owningTerm;

    bc_reserve_size(writer, 0);
    writer->data->localsCount = branch->localsCount;
    writer->data->branch = branch;

    // Check if parent function has a writeNestedBytecode
    if (parent != NULL) {
        FunctionAttrs::WriteNestedBytecode func =
            get_function_attrs(parent->function)->writeNestedBytecode;
        if (func != NULL) {
            func(writer, parent);
            return;
        }
    }

    // Default behavior
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (term == NULL)
            continue;
        bc_call(writer, term);
    }

    bc_stop(writer);
}

void evaluate_branch_with_bytecode(EvalContext* context, Branch* branch)
{
    update_bytecode_for_branch(branch);
    evaluate_bytecode(context, branch->bytecode);
}

void null_bytecode_writer(BytecodeWriter*,Term*) { }

} // namespace circa
