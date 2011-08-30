// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "bytecode.h"
#include "evaluation.h"
#include "function.h"
#include "introspection.h"
#include "refactoring.h"
#include "term.h"

namespace circa {

const int NEW_BYTECODE_DEFAULT_LENGTH = 10;

void print_bytecode(Operation* op, std::ostream& out)
{
    switch (op->type) {
        case OP_CALL: {
            OpCall* cop = (OpCall*) op;
            out << "call " << global_id(cop->term);
            break;
        }
#if 0
        case OP_JUMP: {
            JumpOp* jop = (JumpOp*) op;
            out << "jump " << jop->offset;
            break;
        }
        case OP_JUMPIF: {
            JumpIfOp* jop = (JumpIfOp*) op;
            out << "jumpif " << global_id(jop->input) << " " << jop->offset;
            break;
        }
        case OP_JUMPIFN: {
            JumpIfOp* jop = (JumpIfOp*) op;
            out << "jumpif " << global_id(jop->input) << " " << jop->offset;
            break;
        }
        case OP_CALL: {
            JumpIfNotOp* jop = (JumpIfNotOp*) op;
            out << "jumpifn " << global_id(jop->input) << " " << jop->offset;
            break;
        }
        case OP_BRANCH: {
            BranchOp* bop = (BranchOp*) op;
            out << "branch " << global_id(bop->branchTerm);
            break;
        }
#endif
    }
}

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
static void bytecode_reserve_size(BytecodeWriter* writer, int opCount)
{
    if (writer->listLength >= opCount)
        return;

    int newLength = opCount;

    bool creatingNewData = writer->data == NULL;

    if (creatingNewData)
        newLength = std::max(newLength, NEW_BYTECODE_DEFAULT_LENGTH);

    writer->data = (BytecodeData*) realloc(writer->data,
            sizeof(BytecodeData) + sizeof(AnyOperation) * newLength);
    writer->listLength = newLength;

    if (creatingNewData)
        writer->data->operationCount = 0;
}

// Appends a slot for an operation, returns the operation's index.
int bytecode_append_op(BytecodeWriter* writer)
{
    int pos = writer->writePosition++;
    bytecode_reserve_size(writer, writer->writePosition);
    return pos;
}

int bytecode_call(BytecodeWriter* writer, Term* term, EvaluateFunc func)
{
    int pos = bytecode_append_op(writer);
    OpCall* op = (OpCall*) &writer->data->operations[pos];
    op->type = OP_CALL;
    op->term = term;
    op->func = func;
    return pos;
}

int bytecode_return(BytecodeWriter* writer)
{
    int pos = bytecode_append_op(writer);
    Operation* op = (Operation*) &writer->data->operations[pos];
    op->type = OP_RETURN;
    return pos;
}
int bytecode_return_if_interrupted(BytecodeWriter* writer)
{
    int pos = bytecode_append_op(writer);
    Operation* op = (Operation*) &writer->data->operations[pos];
    op->type = OP_RETURN_IF_INTERRUPTED;
    return pos;
}

void write_bytecode_for_term(BytecodeWriter* writer, Term* term)
{
    // NULL function: no bytecode
    if (term->function == NULL)
        return;

    // Function isn't a function: no bytecode
    if (!is_function(term->function))
        return;

    bytecode_call(writer, term, derive_evaluate_func(term));

    // Backwards compatibility: Always check for interrupted branch
    bytecode_return_if_interrupted(writer);
}

void update_bytecode_for_branch(Branch* branch)
{
    // Don't update if bytecode is not dirty.
    if (branch->bytecode != NULL && !branch->bytecode->dirty)
        return;

    BytecodeWriter writer;
    start_bytecode_update(branch, &writer);

    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (term == NULL)
            continue;
        write_bytecode_for_term(&writer, term);
    }

    // Finish up with a final return call.
    bytecode_return(&writer);

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

        Operation* op = (Operation*) &bytecode->operations[pic];

        switch (op->type) {
        case OP_CALL: {
            OpCall* cop = (OpCall*) op;
            evaluate_single_term(context, cop->term);
            pic++;
            continue;
        }

        case OP_RETURN:
            return;

        case OP_RETURN_ON_ERROR:
            if (context->errorOccurred)
                return;
            continue;

        case OP_RETURN_IF_INTERRUPTED:
            if (evaluation_interrupted(context))
                return;
            continue;

        default:
            internal_error("in evaluate_bytecode, unrecognized op type");
        }
    }
}

void evaluate_branch_with_bytecode(EvalContext* context, Branch* branch)
{
    update_bytecode_for_branch(branch);
    evaluate_bytecode(context, branch->bytecode);
}

} // namespace circa
