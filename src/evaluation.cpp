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
        copy(get_local(context, 0, branch[branch.length()-1]), output);

    pop_stack_frame(context);
}

void evaluate_branch_internal_with_state(EvalContext* context, Term* term,
        Branch& branch)
{
    // Store currentScopeState and fetch the container for this branch
    push_scope_state_for_term(context, term);

    evaluate_branch_internal(context, branch);

    // Store container and replace currentScopeState
    save_and_pop_scope_state(context, term);
}

void evaluate_branch(EvalContext* context, Branch& branch)
{
    push_stack_frame(context, &branch);
    push_scope_state(context);
    Dict::lazyCast(&context->state);
    copy(&context->state, get_current_scope_state(context));

    evaluate_branch_with_bytecode(context, &branch);

    if (context->preserveLocals)
        copy_locals_to_terms(context, branch);

    pop_stack_frame(context);

    swap(get_current_scope_state(context), &context->state);
    pop_scope_state(context);
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
        TaggedValue* val = get_local(context, 0, term);
        if (val != NULL)
            copy(val, branch[i]);
    }
}

void evaluate_save_locals(Branch& branch)
{
    EvalContext context;
    evaluate_save_locals(&context, branch);
}

TaggedValue* get_input(EvalContext* context, Operation* op, int index)
{
    Operation* inputOp = op + 1 + index;

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
    case OP_INPUT_INT:
        internal_error("Can't access INPUT_INT from get_input()");
    default:
        internal_error("not an input instruction");
    }
    return NULL;
}

TaggedValue* get_input(EvalContext* context, OpCall* op, int index)
{
    return get_input(context, (Operation*) op, index);
}

int get_int_input(EvalContext* context, OpCall* op, int index)
{
    Operation* inputOp = ((Operation*) op) + 1 + index;

    if (inputOp->type == OP_INPUT_INT) {
        OpInputInt* iop = (OpInputInt*) inputOp;
        return iop->value;
    } else {
        return as_int(get_input(context, op, index));
    }
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

TaggedValue* get_output(EvalContext* context, Term* term)
{
    TaggedValue* frame = get_stack_frame(context, 0);
    return list_get_index(frame, term->index);
}

TaggedValue* get_output(EvalContext* context, OpCall* op)
{
    TaggedValue* frame = get_stack_frame(context, 0);
    return list_get_index(frame, op->outputIndex);
}

TaggedValue* get_extra_output(EvalContext* context, Term* term, int index)
{
    TaggedValue* frame = get_stack_frame(context, 0);
    return list_get_index(frame, term->index + 1 + index);
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

TaggedValue* get_local(EvalContext* cxt, int relativeFrame, Term* term)
{
    return list_get_index(cxt->stack.getFromEnd(relativeFrame), term->index);
}

void error_occurred(EvalContext* context, Term* errorTerm, std::string const& message)
{
    // Save the error on the context
    TaggedValue* errorValue = get_local(context, 0, errorTerm);
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
    ca_assert(cxt->stateStack.length() > 0);
    return Dict::checkCast(cxt->stateStack.getFromEnd(0));
}
Dict* get_scope_state(EvalContext* cxt, int frame)
{
    return Dict::lazyCast(cxt->stateStack.getFromEnd(frame));
}
void push_scope_state(EvalContext* cxt)
{
    set_dict(cxt->stateStack.append());
}
void pop_scope_state(EvalContext* cxt)
{
    ca_assert(cxt->stateStack.length() > 0);
    cxt->stateStack.pop();
}
void push_scope_state_for_term(EvalContext* cxt, Term* term)
{
    TaggedValue* currentScope = cxt->stateStack.append();
    Dict* prevScope = Dict::lazyCast(cxt->stateStack.getFromEnd(1));
    fetch_state_container(term, prevScope, currentScope);
}

void fetch_state_container(Term* term, TaggedValue* container, TaggedValue* output)
{
    Dict* containerDict = Dict::lazyCast(container);
    copy(containerDict->insert(term->uniqueName.name.c_str()), output);
}
void consume_scope_state_field(Term* term, Dict* scopeState, TaggedValue* output)
{
    TaggedValue* value = scopeState->get(term->uniqueName.name.c_str());
    if (value == NULL) {
        set_null(output);
        return;
    }
    swap(value, output);
    set_null(value);
}
void save_and_pop_scope_state(EvalContext* cxt, Term* term)
{
    Dict* prevScope = Dict::lazyCast(cxt->stateStack.getFromEnd(1));
    Dict* currentScope = Dict::lazyCast(cxt->stateStack.getFromEnd(0));
    save_and_consume_state(term, prevScope, currentScope);
    pop_scope_state(cxt);
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
    // Genrate bytecode for this range.
    BytecodeWriter bytecode;

    for (int i=start; i < end; i++)
        bc_call(&bytecode, branch[i]);
    bc_finish(&bytecode);

    // Now go back and slightly rewrite the bytecode. Any calls that expect a local-value
    // input from a term outside our range, should instead use the term's global value.
    int stackDepth = 0;
    
    for (int i=0; i < bytecode.data->operationCount; i++) {
        Operation* op = &bytecode.data->operations[i];
        if (op->type == OP_INPUT_LOCAL) {
            OpInputLocal* lop = (OpInputLocal*) op;

            bool shouldRemap = false;
            
            if (lop->relativeFrame > stackDepth)
                shouldRemap = true;

            if ((lop->relativeFrame == stackDepth)
                    && (lop->index < start || lop->index >= end))
                shouldRemap = true;

            if (shouldRemap) {
                // convert this input instruction to a global input;
                OpInputGlobal* gop = (OpInputGlobal*) op;
                gop->type = OP_INPUT_GLOBAL;
                Branch* inputBranch = get_parent_branch(branch, lop->relativeFrame - stackDepth);
                Term* global = inputBranch->get(lop->index);
                assert_valid_term(global);
                gop->value = global;
            }
        } else if (op->type == OP_CALL_BRANCH) {
            stackDepth++;
        } else if (op->type == OP_POP_STACK) {
            stackDepth--;
        }
    }

    // Run bytecode
    push_stack_frame(context, &branch);
    push_scope_state(context);
    evaluate_bytecode(context, bytecode.data);

    // Copy locals back to terms
    for (int i=start; i < end; i++) {
        Term* term = branch[i];
        if (is_value(term))
            continue;
        TaggedValue* value = get_local(context, 0, term);
        if (value == NULL)
            continue;
        copy(value, term);
    }

    pop_stack_frame(context);
    pop_scope_state(context);
}

void push_stack_frame(EvalContext* context, int size)
{
    ca_assert(context != NULL);
    set_list(context->stack.append(), size);
}

void push_stack_frame(EvalContext* context, Branch* branch)
{
    ca_assert(branch != NULL);
    push_stack_frame(context, branch->length());
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
            bc_call(&bytecode, branch[i]);
    }
    bc_finish(&bytecode);

    // Run our bytecode
    push_stack_frame(context, &branch);
    evaluate_bytecode(context, bytecode.data);

    // Possibly save output
    if (result != NULL)
        copy(get_local(context, 0, term), result);

    // Clean up
    delete[] marked;

    if (context->preserveLocals)
        copy_locals_to_terms(context, branch);

    pop_stack_frame(context);
}

void evaluate_single_term_with_bytecode(EvalContext* context, Term* term)
{
    BytecodeWriter bytecode;
    bc_call(&bytecode, term);
    bc_finish(&bytecode);

    evaluate_bytecode(context, bytecode.data);
}

void evaluate(EvalContext* context, Branch& branch, std::string const& input)
{
    int prevHead = branch.length();
    parser::compile(branch, parser::statement_list, input);
    evaluate_range(context, branch, prevHead, branch.length());
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
    evaluate_range(&context, branch, prevHead, branch.length());
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
