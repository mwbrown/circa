// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "build_options.h"
#include "builtins.h"
#include "branch.h"
#include "bytecode.h"
#include "code_iterators.h"
#include "evaluation.h"
#include "function.h"
#include "introspection.h"
#include "list_shared.h"
#include "locals.h"
#include "parser.h"
#include "refactoring.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"
#include "types/dict.h"

namespace circa {

void evaluate_single_term(EvalContext* context, OpCall* op)
{
    #if CIRCA_THROW_ON_ERROR
    try {
    #endif

    op->func(context, op);

    #if CIRCA_THROW_ON_ERROR
    } catch (std::exception const& e) { return error_occurred(context, op->term, e.what()); }
    #endif

    // For a test build, we check the type of the output of every single call. This is
    // slow, and it should be unnecessary if the function is written correctly. But it's
    // a good test.
    #ifdef CIRCA_TEST_BUILD
    if (!context->errorOccurred && !is_value(op->term)) {
        Term* term = op->term;
        for (int i=0; i < get_output_count(term); i++) {

            Type* outputType = get_output_type(term, i);
            TaggedValue* output = get_output(context, term, i);

            // Special case, if the function's output type is void then we don't care
            // if the output value is null or not.
            if (i == 0 && outputType == &VOID_T)
                continue;

            if (!cast_possible(output, outputType)) {
                std::stringstream msg;
                msg << "Function " << term->function->name << " produced output "
                    << output->toString() << " (in index " << i << ")"
                    << " which doesn't fit output type "
                    << outputType->name;
                internal_error(msg.str());
            }
        }
    }
    #endif
}

void evaluate_branch_internal(EvalContext* context, Branch& branch)
{
    push_stack_frame(context, &branch);
    evaluate_branch_with_bytecode(context, &branch);

    if (context->preserveLocals)
        copy_locals_to_terms(context, branch);

    pop_stack_frame(context);
}

void evaluate_branch_internal(EvalContext* context, Branch& branch, TaggedValue* output)
{
    push_stack_frame(context, &branch);

    evaluate_branch_with_bytecode(context, &branch);

    if (output != NULL)
        copy(get_local(context, 0, branch[branch.length()-1], 0), output);

    pop_stack_frame(context);
}

void evaluate_branch_internal_with_state(EvalContext* context, Term* term,
        Branch& branch)
{
    // Store currentScopeState and fetch the container for this branch
    TaggedValue prevScopeState;
    swap(&context->currentScopeState, &prevScopeState);
    fetch_state_container(term, &prevScopeState, &context->currentScopeState);

    evaluate_branch_internal(context, branch);

    // Store container and replace currentScopeState
    save_and_consume_state(term, &prevScopeState, &context->currentScopeState);
    swap(&context->currentScopeState, &prevScopeState);
}

void evaluate_branch(EvalContext* context, Branch& branch)
{
    push_stack_frame(context, &branch);
    copy(&context->state, &context->currentScopeState);

    evaluate_branch_with_bytecode(context, &branch);

    if (context->preserveLocals)
        copy_locals_to_terms(context, branch);

    pop_stack_frame(context);

    swap(&context->currentScopeState, &context->state);
    set_null(&context->currentScopeState);
}

void evaluate_save_locals(EvalContext* context, Branch& branch)
{
    context->preserveLocals = true;
    evaluate_branch(context, branch);
}

void copy_locals_to_terms(EvalContext* context, Branch& branch)
{
    // Copy locals back to the original terms. Many tests depend on this functionality.
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (is_value(term)) continue;
        TaggedValue* val = get_local(context, 0, term, 0);
        if (val != NULL)
            copy(val, branch[i]);
    }
}

void evaluate_save_locals(Branch& branch)
{
    EvalContext context;
    context.preserveLocals = true;
    evaluate_save_locals(&context, branch);
}

TaggedValue* get_input(EvalContext* context, OpCall* op, int index)
{
    Operation* opList = (Operation*) op;
    Operation* inputOp = &opList[index + 1];

    switch (inputOp->type) {
    case OP_INPUT_GLOBAL: {
        OpInputGlobal* gop = (OpInputGlobal*) inputOp;
        return gop->value;
    }
    case OP_INPUT_LOCAL: {
        OpInputLocal* lop = (OpInputLocal*) inputOp;
        TaggedValue* frame = get_stack_frame(context, lop->relativeFrame);
        return list_get_index(frame, lop->index);
    }
    case OP_INPUT_NULL:
        return NULL;
    default:
        internal_error("not an input instruction");
    }
    return NULL;
}

TaggedValue* get_input(EvalContext* context, Term* term, int index)
{
    InputInstruction *instruction = &term->inputIsns.inputs[index];

    switch (instruction->type) {
    case InputInstruction::GLOBAL:
        return (TaggedValue*) term->input(index);
    case InputInstruction::EMPTY:
        return NULL;
    case InputInstruction::LOCAL: {
        ca_assert(instruction->relativeFrame >= 0);
        TaggedValue* frame = get_stack_frame(context, instruction->relativeFrame);
        return list_get_index(frame, instruction->index);
    }
    case InputInstruction::LOCAL_CONSUME:
    default:
        internal_error("Not yet implemented");
        return NULL;
    }
}

void consume_input(EvalContext* context, Term* term, int index, TaggedValue* dest)
{
    // Temp, don't actually consume
    copy(get_input(context, term, index), dest);
}

TaggedValue* get_output(EvalContext* context, Term* term, int index)
{
    TaggedValue* frame = get_stack_frame(context, 0);
    return list_get_index(frame, term->localsIndex + index);
}

TaggedValue* get_extra_output(EvalContext* context, Term* term, int index)
{
    return get_output(context, term, index + 1);
}

TaggedValue* get_state_input(EvalContext* cxt, Term* term)
{
    if (term->input(0) == NULL) {
        Dict* currentScopeState = get_current_scope_state(cxt);
        ca_assert(currentScopeState != NULL);
        return currentScopeState->insert(term->uniqueName.name.c_str());
    } else {
        return get_input(cxt, term, 0);
    }
}

TaggedValue* get_local(EvalContext* cxt, int relativeFrame, int index)
{
    return list_get_index(cxt->stack.getFromEnd(relativeFrame), index);
}

TaggedValue* get_local(EvalContext* cxt, int relativeFrame, Term* term, int outputIndex)
{
    int index = term->localsIndex + outputIndex;

    return list_get_index(cxt->stack.getFromEnd(relativeFrame), index);
}

void error_occurred(EvalContext* context, Term* errorTerm, std::string const& message)
{
    // Save the error on the context
    TaggedValue* errorValue = get_local(context, 0, errorTerm, 0);
    set_string(errorValue, message);
    errorValue->value_type = &ERROR_T;
    copy(errorValue, &context->errorValue);

    // Check if there is an errored() call listening to this term. If so, then
    // continue execution.
    if (has_an_error_listener(errorTerm))
        return;

    if (DEBUG_TRAP_ERROR_OCCURRED)
        ca_assert(false);

    ca_assert(errorTerm != NULL);

    if (context == NULL)
        throw std::runtime_error(message);

    if (!context->errorOccurred) {
        context->errorOccurred = true;
        context->errorTerm = errorTerm;
    }
}
void print_runtime_error_formatted(EvalContext& context, std::ostream& output)
{
    output << get_short_location(context.errorTerm)
        << " " << context_get_error_message(&context);
}

Dict* get_current_scope_state(EvalContext* cxt)
{
    return Dict::lazyCast(&cxt->currentScopeState);
}

void fetch_state_container(Term* term, TaggedValue* container, TaggedValue* output)
{
    Dict* containerDict = Dict::lazyCast(container);
    copy(containerDict->insert(term->uniqueName.name.c_str()), output);
}

void save_and_consume_state(Term* term, TaggedValue* container, TaggedValue* result)
{
    Dict* containerDict = Dict::lazyCast(container);
    const char* name = term->uniqueName.name.c_str();
    swap(result, containerDict->insert(name));
    set_null(result);
}

bool evaluation_interrupted(EvalContext* context)
{
    return context->errorOccurred || context->interruptSubroutine
        || context->forLoopContext.breakCalled || context->forLoopContext.continueCalled;
}

void evaluate_range(EvalContext* context, Branch& branch, int start, int end)
{
    BytecodeWriter bytecode;

    for (int i=start; i <= end; i++)
        write_bytecode_for_term(&bytecode, branch[i]);
    bytecode_return(&bytecode);

    push_stack_frame(context, &branch);
    evaluate_bytecode(context, bytecode.data);

    // copy locals back to terms
    for (int i=start; i <= end; i++) {
        Term* term = branch[i];
        if (is_value(term))
            continue;
        TaggedValue* value = get_local(context, 0, term, 0);
        if (value == NULL)
            continue;
        copy(value, term);
    }

    pop_stack_frame(context);
}

void push_stack_frame(EvalContext* context, int size)
{
    set_list(context->stack.append(), size);
}

void push_stack_frame(EvalContext* context, Branch* branch)
{
    push_stack_frame(context, get_locals_count(*branch));
}

void pop_stack_frame(EvalContext* context)
{
    context->stack.pop();
}

List* get_stack_frame(EvalContext* context, int relativeFrame)
{
    return (List*) list_get_index_from_end(&context->stack, relativeFrame);
}

void evaluate_minimum(EvalContext* context, Term* term, TaggedValue* result)
{
    // Short-circuit if the term is a value
    if (is_value(term)) {
        if (result != NULL)
            copy(term, result);
        return;
    }

    Branch& branch = *term->owningBranch;

    // Walk upwards, and "mark" every term that this term depends on. Limit this
    // search to the current branch.

    bool *marked = new bool[branch.length()];
    memset(marked, false, sizeof(bool)*branch.length());

    marked[term->index] = true;

    for (int i=term->index; i >= 0; i--) {
        Term* checkTerm = branch[i];
        if (checkTerm == NULL)
            continue;

        if (marked[i]) {
            for (int inputIndex=0; inputIndex < checkTerm->numInputs(); inputIndex++) {
                Term* input = checkTerm->input(inputIndex);
                if (input == NULL)
                    continue;
                if (input->owningBranch != &branch)
                    continue;
                // don't follow :meta inputs
                if (function_get_input_meta(get_function_attrs(checkTerm->function),
                            inputIndex))
                    continue;
                marked[input->index] = true;
            }
        }
    }

    // Create bytecode for all the marked terms.
    BytecodeWriter bytecode;

    for (int i=0; i <= term->index; i++) {
        if (marked[i])
            write_bytecode_for_term(&bytecode, branch[i]);
    }
    bytecode_return(&bytecode);

    // Run our bytecode
    push_stack_frame(context, &branch);
    evaluate_bytecode(context, bytecode.data);

    // Possibly save output
    if (result != NULL)
        copy(get_local(context, 0, term, 0), result);

    // Clean up
    delete[] marked;

    if (context->preserveLocals)
        copy_locals_to_terms(context, branch);

    pop_stack_frame(context);
}

void evaluate_single_term_with_bytecode(EvalContext* context, Term* term)
{
    BytecodeWriter bytecode;
    write_bytecode_for_term(&bytecode, term);
    bytecode_return(&bytecode);

    evaluate_bytecode(context, bytecode.data);
}

void evaluate(EvalContext* context, Branch& branch, std::string const& input)
{
    int prevHead = branch.length();
    parser::compile(branch, parser::statement_list, input);
    evaluate_range(context, branch, prevHead, branch.length() - 1);
}

void evaluate(Branch& branch, Term* function, List* inputs)
{
    EvalContext context;

    TermList inputTerms;
    inputTerms.resize(inputs->length());

    for (int i=0; i < inputs->length(); i++)
        inputTerms.setAt(i, create_value(branch, inputs->get(i)));

    int prevHead = branch.length();
    apply(branch, function, inputTerms);
    evaluate_range(&context, branch, prevHead, branch.length() - 1);
}

void evaluate(Term* function, List* inputs)
{
    Branch scratch;
    return evaluate(scratch, function, inputs);
}

void clear_error(EvalContext* cxt)
{
    cxt->errorOccurred = false;
    cxt->errorTerm = NULL;
}

std::string context_get_error_message(EvalContext* cxt)
{
    ca_assert(cxt != NULL);
    ca_assert(cxt->errorTerm != NULL);
    return as_string(&cxt->errorValue);
}

} // namespace circa
