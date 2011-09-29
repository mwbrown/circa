// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "build_options.h"
#include "branch.h"
#include "code_iterators.h"
#include "evaluation.h"
#include "function.h"
#include "interpreter.h"
#include "introspection.h"
#include "list_shared.h"
#include "kernel.h"
#include "locals.h"
#include "parser.h"
#include "refactoring.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"
#include "types/dict.h"

namespace circa {

EvalContext::~EvalContext()
{
    while (numFrames > 0)
        pop_frame(this);

    free(frames);
    frames = NULL;
}

void evaluate_branch_internal(EvalContext* context, Branch* branch)
{
#if 0
    push_stack_frame(context, branch);
    evaluate_branch_with_bytecode(context, branch);

    if (context->preserveLocals)
        copy_locals_to_terms(context, branch);

    pop_stack_frame(context);
#endif
}

void evaluate_branch_internal(EvalContext* context, Branch* branch, TaggedValue* output)
{
#if 0
    push_stack_frame(context, branch);

    evaluate_branch_with_bytecode(context, branch);

    if (output != NULL)
        copy(get_local(context, 0, branch->get(branch->length()-1)), output);

    pop_stack_frame(context);
#endif
}

void evaluate_branch_internal_with_state(EvalContext* context, Term* term,
        Branch* branch)
{
    // Store currentScopeState and fetch the container for this branch
    push_scope_state_for_term(context, term);

    evaluate_branch_internal(context, branch);

    // Store container and replace currentScopeState
    save_and_pop_scope_state(context, term);
}

void evaluate_branch(EvalContext* context, Branch* branch)
{
    interpret(context, branch);
#if 0
    push_stack_frame(context, branch);
    push_scope_state(context);
    Dict::lazyCast(&context->state);
    copy(&context->state, get_current_scope_state(context));

    evaluate_branch_with_bytecode(context, branch);

    if (context->preserveLocals)
        copy_locals_to_terms(context, branch);

    pop_stack_frame(context);

    swap(get_current_scope_state(context), &context->state);
    pop_scope_state(context);
#endif
}

void evaluate_save_locals(EvalContext* context, Branch* branch)
{
    context->preserveLocals = true;
    evaluate_branch(context, branch);
}

void evaluate_save_locals(Branch* branch)
{
    EvalContext context;
    evaluate_save_locals(&context, branch);
}
void evaluate(EvalContext* context, Branch* branch, std::string const& input)
{
    // FIXME
}
void evaluate(Branch* branch, Term* function, List* inputs)
{
    // FIXME
}
void evaluate(Term* function, List* inputs)
{
    // FIXME
}
void error_occurred(EvalContext* context, Term* errorTerm, std::string const& message)
{
    TaggedValue errorValue;

    set_string(&errorValue, message);
    errorValue.value_type = &ERROR_T;

    // Save error on context
    copy(&errorValue, &context->errorValue);

    // If possible, save error as the term's output.
    TaggedValue* local = get_output_safe(context, errorTerm);
    if (local != NULL)
        copy(&errorValue, local);

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

void evaluate_minimum(EvalContext* context, Term* term, TaggedValue* result)
{
    //FIXME
}
void evaluate_range(EvalContext* context, Branch* branch, int start, int end)
{
    interpret_range(context, branch, start, end);
}

} // namespace circa
