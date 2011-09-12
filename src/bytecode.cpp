// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "builtins.h"
#include "bytecode.h"
#include "evaluation.h"
#include "function.h"
#include "introspection.h"
#include "locals.h"
#include "refactoring.h"
#include "term.h"

namespace circa {

const int NEW_BYTECODE_DEFAULT_LENGTH = 6;

static bool is_input_op_type(OpType type)
{
    switch (type) {
        case OP_INPUT_LOCAL:
        case OP_INPUT_GLOBAL:
        case OP_INPUT_NULL:
            return true;
        default:
            return false;
    }
}

void print_bytecode_op(Operation* op, int loc, std::ostream& out)
{
    switch (op->type) {
        case OP_CALL:
            out << "call " << get_unique_name(((OpCall*) op)->term);
            break;
        case OP_INPUT_NULL:
            out << "input_null";
            break;
        case OP_INPUT_GLOBAL: {
            OpInputGlobal* gop = (OpInputGlobal*) op;
            out << "input_global ";
            if (gop->value == NULL)
               out << "<NULL>";
            else
               out << gop->value->toString();
            break;
        }
        case OP_INPUT_LOCAL: {
            OpInputLocal* lop = (OpInputLocal*) op;
            out << "input_local frame:" << lop->relativeFrame << " idx:" << lop->index;
            break;
        }

        case OP_RETURN:
            out << "return";
            break;
        case OP_RETURN_ON_ERROR:
            out << "return_on_error";
            break;
        case OP_STACK_SIZE:
            out << "stack_size " << ((OpStackSize*) op)->size;
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
        case OP_CALL_BRANCH:
            out << "call_branch " << global_id(((OpCallBranch*) op)->term);
            break;
        case OP_POP_STACK:
            out << "pop_stack";
            break;
        case OP_COPY:
            out << "copy";
            break;
        default:
            out << "<unknown opcode " << int(op->type) << ">";
    }
}

void print_bytecode(BytecodeData* bytecode, std::ostream& out)
{
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

        print_bytecode_op(&bytecode->operations[i], i, out);
    }
    out << std::endl;
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
static void bc_reserve_size(BytecodeWriter* writer, int opCount)
{
    if (writer->listLength >= opCount)
        return;

    int newLength = opCount;
    
    if (writer->data == NULL) {
        newLength = std::max(newLength, NEW_BYTECODE_DEFAULT_LENGTH);
        writer->data = (BytecodeData*) malloc(
                sizeof(BytecodeData) + sizeof(Operation) * newLength);
        writer->data->operationCount = 0;
        writer->data->dirty = false;
    } else {
        writer->data = (BytecodeData*) realloc(writer->data,
                sizeof(BytecodeData) + sizeof(Operation) * newLength);
    }

    writer->listLength = newLength;
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

    // Write information for each input
    for (int i=0; i < term->numInputs(); i++) {
        Term* input = term->input(i);
        if (input == NULL) {
            bc_append_op(writer)->type = OP_INPUT_NULL;
            continue;
        }

        Operation* inputOp = bc_append_op(writer);

        // Use InputOverride if there is one
        if (writer->inputOverride != NULL) {
            // write NULL to the type. If the override leaves it as NULL then we'll
            // continue on to the default behavior.
            inputOp->type = OP_INPUT_NULL;
            writer->inputOverride(writer->inputOverrideContext, input, inputOp);

            if (inputOp->type != OP_INPUT_NULL) {
                ca_assert(inputOp->type == OP_INPUT_GLOBAL || inputOp->type == OP_INPUT_LOCAL);
                continue;
            }
        }

        if (is_value(input)) {
            bc_write_global_input(inputOp, (TaggedValue*) input);
        } else {
            int index = input->index;

            // Fun special case for for-loop locals
            if (input->function == JOIN_FUNC && get_parent_term(input)->name == "#inner_rebinds")
                index = 1 + input->index;

            int relativeFrame = get_frame_distance(term, input);
            bc_write_local_input(inputOp, relativeFrame, index);
        }
    }
}

void bc_return(BytecodeWriter* writer)
{
    bc_append_op(writer)->type = OP_RETURN;
}
void bc_imaginary_call(BytecodeWriter* writer, EvaluateFunc func)
{
    OpCall* op = (OpCall*) bc_append_op(writer);
    op->type = OP_CALL;
    op->term = NULL;
    op->func = func;
}
void bc_imaginary_call(BytecodeWriter* writer, Term* func)
{
    bc_imaginary_call(writer, get_function_attrs(func)->evaluate);
}
int bc_jump(BytecodeWriter* writer)
{
    int result = writer->writePosition;
    OpJump* op = (OpJump*) bc_append_op(writer);
    op->type = OP_JUMP;
    op->offset = 0;
    return result;
}
int bc_jump_if(BytecodeWriter* writer)
{
    int result = writer->writePosition;
    OpJump* op = (OpJump*) bc_append_op(writer);
    op->type = OP_JUMP_IF;
    op->offset = 0;
    return result;
}
int bc_jump_if_not(BytecodeWriter* writer)
{
    int result = writer->writePosition;
    OpJump* op = (OpJump*) bc_append_op(writer);
    op->type = OP_JUMP_IF_NOT;
    op->offset = 0;
    return result;
}
int bc_jump_if_not_equal(BytecodeWriter* writer)
{
    int result = writer->writePosition;
    OpJump* op = (OpJump*) bc_append_op(writer);
    op->type = OP_JUMP_IF_NOT_EQUAL;
    op->offset = 0;
    return result;
}

void bc_jump_to_here(BytecodeWriter* writer, int jumpPos)
{
    OpJump* op = (OpJump*) &writer->data->operations[jumpPos];
    ca_assert(op->type == OP_JUMP_IF || op->type == OP_JUMP_IF_NOT
            || op->type == OP_JUMP || op->type == OP_JUMP_IF_NOT_EQUAL);
    op->offset = writer->writePosition - jumpPos;
}
void bc_global_input(BytecodeWriter* writer, TaggedValue* value)
{
    OpInputGlobal* gop = (OpInputGlobal*) bc_append_op(writer);
    gop->type = OP_INPUT_GLOBAL;
    gop->value = (TaggedValue*) value;
}
void bc_local_input(BytecodeWriter* writer, int frame, int index)
{
    OpInputLocal *lop = (OpInputLocal*) bc_append_op(writer);
    lop->type = OP_INPUT_LOCAL;
    lop->relativeFrame = frame;
    lop->index = index;
}
void bc_write_input(BytecodeWriter* writer, Branch* frame, Term* input)
{
    if (input == NULL) {
        bc_append_op(writer)->type = OP_INPUT_NULL;
        return;
    }

    Operation* inputOp = bc_append_op(writer);

    // Use InputOverride if there is one
    if (writer->inputOverride != NULL) {
        // write NULL to the type. If the override leaves it as NULL then we'll
        // continue on to the default behavior.
        inputOp->type = OP_INPUT_NULL;
        writer->inputOverride(writer->inputOverrideContext, input, inputOp);

        if (inputOp->type != OP_INPUT_NULL) {
            ca_assert(inputOp->type == OP_INPUT_GLOBAL || inputOp->type == OP_INPUT_LOCAL);
            return;
        }
    }

    if (is_value(input)) {
        bc_write_global_input(inputOp, (TaggedValue*) input);
    } else {
        int relativeFrame = get_frame_distance(frame, input);
        bc_write_local_input(inputOp, relativeFrame, input->index);
    }
}
void bc_write_global_input(Operation* op, TaggedValue* value)
{
    OpInputGlobal* gop = (OpInputGlobal*) op;
    gop->type = OP_INPUT_GLOBAL;
    gop->value = (TaggedValue*) value;
}
void bc_write_local_input(Operation* op, int frame, int index)
{
    OpInputLocal *lop = (OpInputLocal*) op;
    lop->type = OP_INPUT_LOCAL;
    lop->relativeFrame = frame;
    lop->index = index;
}
void bc_copy_value(BytecodeWriter* writer)
{
    bc_append_op(writer)->type = OP_COPY;
}
void bc_call_branch(BytecodeWriter* writer, Term* term)
{
    OpCallBranch *cop = (OpCallBranch*) bc_append_op(writer);
    cop->type = OP_CALL_BRANCH;
    cop->term = term;
}
void bc_pop_stack(BytecodeWriter* writer)
{
    bc_append_op(writer)->type = OP_POP_STACK;
}

void dirty_bytecode(Term* term)
{
    if (term->owningBranch != NULL)
        dirty_bytecode(*term->owningBranch);
}

void dirty_bytecode(Branch& branch)
{
    if (branch.bytecode != NULL)
        branch.bytecode->dirty = true;
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
            || term->name == "#outer_rebinds"
            || term->name == "#attributes")
        return;

    // Check if the function has a special writer function
    FunctionAttrs::WriteBytecode writeBytecode =
        get_function_attrs(term->function)->writeBytecode;

    if (writeBytecode != NULL) {
        writeBytecode(term, writer);
        return;
    }

    // Default: Add an OP_CALL
    bc_write_call_op(writer, term, get_function_attrs(term->function)->evaluate);
}

void bc_finish(BytecodeWriter* writer)
{
    bc_return(writer);
}

void bc_reset_writer(BytecodeWriter* writer)
{
    writer->writePosition = 0;
    if (writer->data != NULL)
        writer->data->operationCount = 0;
}

void update_bytecode_for_branch(Branch* branch)
{
    // Don't update if bytecode is not dirty.
    if (branch->bytecode != NULL && !branch->bytecode->dirty)
        return;

    // Deprecated steps:
    update_input_instructions(*branch);

    BytecodeWriter writer;
    start_bytecode_update(branch, &writer);

    Term* parent = branch->owningTerm;

    // TODO: Add a stack_size operation.

    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (term == NULL)
            continue;
        bc_call(&writer, term);
    }

    // Check if the parent function has a bytecodeFinish call
    if (parent != NULL) {
        FunctionAttrs::WriteNestedBytecodeFinish func =
            get_function_attrs(parent->function)->writeNestedBytecodeFinish;
        if (func != NULL)
            func(parent, &writer);
    }

    // Finish up with a final return call.
    bc_finish(&writer);

    finish_bytecode_update(branch, &writer);
}

void evaluate_bytecode(EvalContext* context, BytecodeData* bytecode)
{
    int pc = 0;

    if (bytecode == NULL || bytecode->operationCount == 0)
        return;

    while (true) {

        ca_assert(pc >= 0);
        ca_assert(pc < bytecode->operationCount);

        Operation* op = &bytecode->operations[pc];

        switch (op->type) {
        case OP_CALL: {

            // this will be removed:
            bool evalWasInterrupted = evaluation_interrupted(context);

            OpCall* cop = (OpCall*) op;
            evaluate_single_term(context, cop);
            pc += 1;

            // TODO: Should skip over input instructions when possible

            // this will be removed:
            if (evaluation_interrupted(context) && !evalWasInterrupted)
                return;

            continue;
        }

        case OP_INPUT_LOCAL:
        case OP_INPUT_GLOBAL:
        case OP_INPUT_NULL:
            pc += 1;
            continue;

        case OP_RETURN:
            return;

        case OP_RETURN_ON_ERROR:
            if (context->errorOccurred)
                return;
            pc++;
            continue;

        case OP_STACK_SIZE: {
            List* frame = get_stack_frame(context, 0);
            frame->resize(((OpStackSize*) op)->size);
            pc++;
            continue;
        }

        case OP_JUMP: {
            OpJump* jop = (OpJump*) op;
            ca_assert(jop->offset != 0);
            pc += jop->offset;
            continue;
        }

        case OP_JUMP_IF: {
            OpJump* jop = (OpJump*) op;
            TaggedValue* input = get_input(context, op, 0);
            ca_assert(is_bool(input));
            if (as_bool(input)) {
                ca_assert(jop->offset != 0);
                pc += jop->offset;
            } else {
                pc += 1;
            }
            continue;
        }
        case OP_JUMP_IF_NOT: {
            OpJump* jop = (OpJump*) op;
            TaggedValue* input = get_input(context, op, 0);
            ca_assert(is_bool(input));
            if (!as_bool(input)) {
                ca_assert(jop->offset != 0);
                pc += jop->offset;
            } else {
                pc += 1;
            }
            continue;
        }
        case OP_JUMP_IF_NOT_EQUAL: {
            OpJump* jop = (OpJump*) op;
            TaggedValue* left = get_input(context, op, 0);
            TaggedValue* right = get_input(context, op, 1);
            if (!equals(left,right)) {
                ca_assert(jop->offset != 0);
                pc += jop->offset;
            } else {
                pc += 1;
            }
            continue;
        }

        case OP_CALL_BRANCH: {

            // TODO: Push branch to the stack but continue in this loop
            OpCallBranch* cop = (OpCallBranch*) op;
            Branch* branch = &nested_contents(cop->term);
            push_stack_frame(context, branch);
            evaluate_branch_with_bytecode(context, branch);
            pc += 1;
            continue;
        }
        case OP_POP_STACK: {
            pop_stack_frame(context);
            pc += 1;
            continue;
        }

        case OP_COPY: {
            TaggedValue* left = get_input(context, op, 0);
            TaggedValue* right = get_input(context, op, 1);
            copy(left, right);
            pc += 1;
            continue;
        }

        default:
            internal_error("in evaluate_bytecode, unrecognized op type");
        }
    }
}

void evaluate_branch_with_bytecode(EvalContext* context, Branch* branch)
{
    update_bytecode_for_branch(branch);

    //dump(*branch);
    //print_bytecode(branch->bytecode, std::cout);

    evaluate_bytecode(context, branch->bytecode);
}

void null_bytecode_writer(Term*, BytecodeWriter*)
{
}

} // namespace circa
