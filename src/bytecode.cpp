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

void print_bytecode_op(Operation* op, std::ostream& out)
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
            out << "input_local frame:" << lop->relativeFrame << " index:" << lop->index;
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
        default:
            out << "<unknown opcode>";
    }
}

void print_bytecode(BytecodeData* bytecode, std::ostream& out)
{
    for (int i=0; i < bytecode->operationCount; i++) {
        if (i != 0)
            out << "; ";
        print_bytecode_op(&bytecode->operations[i], out);
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
            int index = input->localsIndex + term->inputInfo(i)->outputIndex;

            // Fun special case for for-loop locals
            if (input->function == JOIN_FUNC && get_parent_term(input)->name == "#inner_rebinds")
                index = 1 + input->localsIndex;

            int relativeFrame = get_frame_distance(term, input);
            bc_write_local_input(inputOp, relativeFrame, index);
        }
    }
}

void bc_return(BytecodeWriter* writer)
{
    bc_append_op(writer)->type = OP_RETURN;
}
OpJumpIf* bc_jump_if(BytecodeWriter* writer)
{
    OpJumpIf* op = (OpJumpIf*) bc_append_op(writer);
    op->type = OP_JUMP_IF;
    op->offset = 0;
    return op;
}
OpJumpIf* bc_jump_if_not(BytecodeWriter* writer)
{
    OpJumpIf* op = (OpJumpIf*) bc_append_op(writer);
    op->type = OP_JUMP_IF_NOT;
    op->offset = 0;
    return op;
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
        bc_write_local_input(inputOp, relativeFrame, input->localsIndex);
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

void update_bytecode_for_branch(Branch* branch)
{
    // Don't update if bytecode is not dirty.
    if (branch->bytecode != NULL && !branch->bytecode->dirty)
        return;

    // Deprecated steps:
    refresh_locals_indices(*branch, 0);
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
    int pic = 0;

    if (bytecode == NULL || bytecode->operationCount == 0)
        return;

    while (true) {

        ca_assert(pic >= 0);
        ca_assert(pic < bytecode->operationCount);

        Operation* op = &bytecode->operations[pic];

        switch (op->type) {
        case OP_CALL: {

            // this will be removed:
            bool evalWasInterrupted = evaluation_interrupted(context);

            OpCall* cop = (OpCall*) op;
            evaluate_single_term(context, cop);
            int numInputs = cop->term->numInputs();
            pic += 1 + numInputs;

            // this will be removed:
            if (evaluation_interrupted(context) && !evalWasInterrupted)
                return;

            continue;
        }

        case OP_INPUT_LOCAL:
        case OP_INPUT_GLOBAL:
        case OP_INPUT_NULL:
            internal_error("bytecode error, interpreter hit an input instruction");
            return;

        case OP_RETURN:
            return;

        case OP_RETURN_ON_ERROR:
            if (context->errorOccurred)
                return;
            pic++;
            continue;

        case OP_STACK_SIZE: {
            List* frame = get_stack_frame(context, 0);
            frame->resize(((OpStackSize*) op)->size);
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
