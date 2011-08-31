// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "builtins.h"
#include "bytecode.h"
#include "evaluation.h"
#include "function.h"
#include "introspection.h"
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
        case OP_CHECK_CALL:
            out << "chcall " << get_unique_name(((OpCall*) op)->term);
            break;
        case OP_RETURN:
            out << "return";
            break;
        case OP_RETURN_ON_ERROR:
            out << "return_on_error";
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
static void bytecode_reserve_size(BytecodeWriter* writer, int opCount)
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
int bytecode_append_op(BytecodeWriter* writer)
{
    int pos = writer->writePosition++;
    bytecode_reserve_size(writer, writer->writePosition);
    writer->data->operationCount++;
    return pos;
}

int bytecode_call(BytecodeWriter* writer, Term* term, EvaluateFunc func)
{
    int pos = bytecode_append_op(writer);
    OpCall* op = (OpCall*) &writer->data->operations[pos];
    op->type = OP_CHECK_CALL;
    op->term = term;
    op->func = func;
    return pos;
}

int bytecode_return(BytecodeWriter* writer)
{
    int pos = bytecode_append_op(writer);
    writer->data->operations[pos].type = OP_RETURN;
    return pos;
}

void dirty_bytecode(Term* term)
{
    if (term->owningBranch != NULL && term->owningBranch->bytecode != NULL)
        term->owningBranch->bytecode->dirty = true;
}

void write_bytecode_for_term(BytecodeWriter* writer, Term* term)
{
    // NULL function: no bytecode
    if (term->function == NULL)
        return;

    // Function isn't a function: no bytecode
    if (!is_function(term->function))
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

    // Default: Add an OP_CHECK_CALL
    bytecode_call(writer, term, get_function_attrs(term->function)->evaluate);
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

        Operation* op = &bytecode->operations[pic];

        switch (op->type) {
        case OP_CALL:
            evaluate_single_term(context, ((OpCall*) op)->term);
            pic++;
            continue;
        case OP_CHECK_CALL: {
            OpCall* cop = (OpCall*) op;
            evaluate_single_term(context, cop->term);
            pic++;
            if (evaluation_interrupted(context))
                return;
            continue;
        }

        case OP_RETURN:
            return;

        case OP_RETURN_ON_ERROR:
            if (context->errorOccurred)
                return;
            pic++;
            continue;

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
