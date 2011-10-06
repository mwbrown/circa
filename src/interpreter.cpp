// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "bytecode.h"
#include "evaluation.h"
#include "interpreter.h"
#include "introspection.h"
#include "list_shared.h"
#include "kernel.h"
#include "locals.h"
#include "function.h"
#include "term.h"
#include "type.h"

namespace circa {

Frame* push_frame(EvalContext* context, Branch* branch, BytecodeData* bytecode)
{
    context->numFrames++;
    context->frames = (Frame*) realloc(context->frames, sizeof(Frame) * context->numFrames);
    Frame* top = &context->frames[context->numFrames-1];
    top->pc = 0;
    top->branch = branch;
    top->locals.initializeNull();
    set_list(&top->locals, branch->length());
    top->state.initializeNull();
    top->temporary.initializeNull();
    set_dict(&top->state);
    top->bytecode = bytecode;
    return top;
}

Frame* push_frame(EvalContext* context, Branch* branch)
{
    return push_frame(context, branch, branch->bytecode);
}

void pop_frame(EvalContext* context)
{
    ca_assert(context->numFrames > 0);

    Frame* top = top_frame(context);
    set_null(&top->locals);
    set_null(&top->state);
    set_null(&top->temporary);
    context->numFrames--;

    if (context->numFrames > 0)
        top_frame(context)->pc++;
}

Frame* top_frame(EvalContext* context)
{
    ca_assert(context->numFrames > 0);
    return &context->frames[context->numFrames-1];
}
Frame* get_frame(EvalContext* context, int frame)
{
    ca_assert(frame < context->numFrames);
    ca_assert(frame >= 0);
    return &context->frames[context->numFrames-1-frame];
}
Term* get_pc_term(EvalContext* context)
{
    Frame* frame = top_frame(context);
    return frame->branch->get(frame->pc);
}
TaggedValue* get_input(EvalContext* context, Term* term, int index)
{
    return get_input_rel(context, term, 0, index);
}
TaggedValue* get_input_rel(EvalContext* context, Term* term, int frameDistance, int index)
{
    Term* input = term->input(index);

    if (input == NULL) {
        return NULL;
    } else if (is_value(input)) {
        return input;
    } else {
        int relativeFrame = get_frame_distance(term->owningBranch, input);
        Frame* frame = get_frame(context, relativeFrame + frameDistance);
        int localsIndex = term->input(index)->index;
        ca_assert(localsIndex < frame->locals.length());
        return frame->locals[term->input(index)->index];
    }
}
void consume_input(EvalContext* context, Term* term, int index, TaggedValue* output)
{
    // TEMP: Don't actually consume
    copy(get_input(context, term, index), output);
}
TaggedValue* get_current_input(EvalContext* context, int index)
{
    Term* term = get_pc_term(context);
    return get_input(context, term, index);
}
TaggedValue* get_output(EvalContext* context, Term* term)
{
    Frame* frame = top_frame(context);
    ca_assert(frame->branch == term->owningBranch);
    return frame->locals[term->index];
}
TaggedValue* get_output_rel(EvalContext* context, Term* term, int frameDistance)
{
    Frame* frame = get_frame(context, frameDistance);
    ca_assert(frame->branch == term->owningBranch);
    return frame->locals[term->index];
}
TaggedValue* get_current_output(EvalContext* context)
{
    Frame* frame = top_frame(context);
    return frame->locals[frame->pc];
}
TaggedValue* get_extra_output(EvalContext* context, Term* term, int index)
{
    Frame* frame = top_frame(context);
    return frame->locals[term->index + 1 + index];
}
TaggedValue* get_extra_output_rel(EvalContext* context, Term* term, int frameDistance, int index)
{
    Frame* frame = get_frame(context, frameDistance);
    return frame->locals[term->index + 1 + index];
}
TaggedValue* get_output_safe(EvalContext* context, Term* term)
{
    // FIXME
    return NULL;
}
TaggedValue* get_local(EvalContext* context, int relativeFrame, int index)
{
    return get_frame(context, relativeFrame)->locals[index];
}

TaggedValue* follow_input_instruction(EvalContext* context, Operation* op)
{
    switch (op->type) {
        case OP_INPUT_GLOBAL: {
            OpInputGlobal* gop = (OpInputGlobal*) op;
            return gop->value;
        }
        case OP_INPUT_LOCAL: {
            OpInputLocal* lop = (OpInputLocal*) op;
            Frame* frame = get_frame(context, lop->relativeFrame);
            return list_get_index(&frame->locals, lop->index);
        }
        case OP_INPUT_NULL:
            return NULL;
    }
    return NULL;
}

void finish_branch(EvalContext* context, int flags)
{
    Frame* frame = top_frame(context);

    // Preserve stateful terms. Good candidate for optimization...
    {
        Branch* branch = frame->branch;
        for (int i=0; i < branch->length(); i++) {
            Term* term = branch->get(i);
            if (term->function == GET_STATE_FIELD_FUNC && term->name != "") {
                Term* outcome = find_name(branch, term->name.c_str());
                TaggedValue* value = get_output(context, outcome);
                copy(value, frame->state.insert(term->name.c_str()));
            }
        }
    }

#if 0
    // Check if the calling function specifies a custom finishBranch handler
    if (frame->finishBranch) {
        bool continueFinish = frame->finishBranch(context, flags);
        if (!continueFinish)
            return;
    }
#endif

    if (context->preserveLocals)
        copy_locals_to_terms(context, frame->branch);

    pop_frame(context);
}

bool top_level_finish_branch(EvalContext* context, int flags)
{
    move(&top_frame(context)->state, &context->state);
    return true;
}

bool check_output_type(EvalContext* context, Term* term)
{
    if (!context->errorOccurred) {

        Type* outputType = declared_type(term);
        TaggedValue* output = get_output(context, term);

        // Special case, if the function's output type is void then we don't care
        // if the output value is null or not.
        if (outputType == &VOID_T)
            ;

        else if (!cast_possible(output, outputType)) {
            std::stringstream msg;
            msg << "term " << global_id(term) << ", function " << term->function->name
                << " produced output "
                << output->toString()
                << " which doesn't fit output type "
                << outputType->name;

            error_occurred(context, term, msg.str());

            return false;
        }
    }
    return true;
}

void interpreter_start(EvalContext* context, Branch* branch)
{
    update_bytecode_for_branch(branch);

    Frame* firstFrame = push_frame(context, branch);
    if (is_dict(&context->state))
        copy(&context->state, &firstFrame->state);
}
void interpreter_start(EvalContext* context, BytecodeData* bytecode)
{
    Frame* firstFrame = push_frame(context, NULL, bytecode);
    if (is_dict(&context->state))
        copy(&context->state, &firstFrame->state);
}

void interpreter_step(EvalContext* context)
{
    Frame* frame = top_frame(context);
    BytecodeData* bytecode = frame->bytecode;

    // Check to finish this branch
    if (frame->pc >= bytecode->operationCount) {

        finish_branch(context, 0);
        return;
    }

    Operation* op = &bytecode->operations[frame->pc];

    switch (op->type) {
    case OP_CALL: {

        TaggedValue* input_pointers[20];
#if CIRCA_TEST_BUILD
        memset(input_pointers, 0xa0a0, sizeof(input_pointers));
#endif
        OpCall* cop = (OpCall*) op;

        // Fetch pointers for input instructions
        int input = 0;
        bool probingInputs = true;
        while (probingInputs) {
            Operation* inputOp = op + 1 + input;
            switch (inputOp->type) {
                case OP_INPUT_GLOBAL: {
                    OpInputGlobal* gop = (OpInputGlobal*) inputOp;
                    input_pointers[input++] = gop->value;
                    break;
                }
                case OP_INPUT_LOCAL: {
                    OpInputLocal* lop = (OpInputLocal*) inputOp;
                    Frame* frame = get_frame(context, lop->relativeFrame);
                    input_pointers[input++] = list_get_index(&frame->locals, lop->index);
                    break;
                }
                case OP_INPUT_NULL:
                    input_pointers[input++] = NULL;
                    break;
                default:
                    probingInputs = false;
            }
        }
        int inputCount = input;

        #if CIRCA_THROW_ON_ERROR
        try {
        #endif

        cop->func(context, inputCount, input_pointers);

        #if CIRCA_THROW_ON_ERROR
        } catch (std::exception const& e) { error_occurred(context, cop->term, e.what()); }
        #endif

        frame->pc += inputCount + 1;

        break;
    }

    case OP_CHECK_OUTPUT: {
        Term* term = ((OpCheckOutput*) op)->term;

        // Call check_output_type, and halt interpreter if it fails.
        if (!check_output_type(context, term))
            return;

        frame->pc += 1;
        break;
    }

    case OP_INPUT_LOCAL:
    case OP_INPUT_GLOBAL:
    case OP_INPUT_NULL:
    case OP_INPUT_INT:
        frame->pc += 1;
        break;

    case OP_RETURN:
        frame->pc += 1;
        return;

    case OP_RETURN_ON_ERROR:
        if (context->errorOccurred)
            return;
        frame->pc++;
        break;

    case OP_JUMP: {
        OpJump* jop = (OpJump*) op;
        ca_assert(jop->offset != 0);
        frame->pc += jop->offset;
        break;
    }

    case OP_JUMP_IF: {
        OpJump* jop = (OpJump*) op;
        TaggedValue* input = follow_input_instruction(context, op+1);
        ca_assert(is_bool(input));
        if (as_bool(input)) {
            ca_assert(jop->offset != 0);
            frame->pc += jop->offset;
        } else {
            frame->pc += 1;
        }
        break;
    }
    case OP_JUMP_IF_NOT: {
        OpJump* jop = (OpJump*) op;
        TaggedValue* input = follow_input_instruction(context, op+1);
        ca_assert(is_bool(input));
        if (!as_bool(input)) {
            ca_assert(jop->offset != 0);
            frame->pc += jop->offset;
        } else {
            frame->pc += 1;
        }
        break;
    }
    case OP_JUMP_IF_NOT_EQUAL: {
        OpJump* jop = (OpJump*) op;
        TaggedValue* left = follow_input_instruction(context, op+1);
        TaggedValue* right = follow_input_instruction(context, op+2);
        if (!equals(left,right)) {
            ca_assert(jop->offset != 0);
            frame->pc += jop->offset;
        } else {
            frame->pc += 1;
        }
        break;
    }
    case OP_JUMP_IF_WITHIN_RANGE: {
        OpJump* jop = (OpJump*) op;
        TaggedValue* list = follow_input_instruction(context, op+1);
        TaggedValue* index = follow_input_instruction(context, op+2);

        bool doJump = as_int(index) < list_get_length(list);

        if (doJump) {
            ca_assert(jop->offset != 0);
            frame->pc += jop->offset;
        } else {
            frame->pc += 1;
        }
        break;
    }

    case OP_CALL_BRANCH: {

        // TODO: Push branch to the stack but continue in this loop
        OpCallBranch* cop = (OpCallBranch*) op;

        Term* termForCallStack = cop->term;
        if (get_parent_term(termForCallStack) != NULL
            && get_parent_term(termForCallStack)->function == IF_BLOCK_FUNC)
            termForCallStack = get_parent_term(termForCallStack);

        Branch* branch = nested_contents(cop->term);
        push_frame(context, branch);
        evaluate_branch_with_bytecode(context, branch);
        frame->pc += 1;

        break;
    }
    case OP_POP_STACK: {
        pop_frame(context);
        frame->pc += 1;
        break;
    }

    case OP_COPY: {
        TaggedValue* left = follow_input_instruction(context, op + 1);
        TaggedValue* right = follow_input_instruction(context, op + 2);
        copy(left, right);

        frame->pc += 1;
        break;
    }

    case OP_INCREMENT: {
        TaggedValue* val = follow_input_instruction(context, op + 1);
        set_int(val, as_int(val) + 1);
        frame->pc += 1;
        break;
    }

    default:
        internal_error("in evaluate_bytecode, unrecognized op type");
    }
}

bool interpreter_finished(EvalContext* context)
{
    return context->numFrames <= 0;
}
void interpreter_halt(EvalContext* context)
{
    while (context->numFrames > 0)
        finish_branch(context, 0);
}

InterpretResult interpret(EvalContext* context, Branch* branch)
{
    interpreter_start(context, branch);

    // Main loop
    while (!interpreter_finished(context))
        interpreter_step(context);

    return SUCCESS;
}
InterpretResult interpret(EvalContext* context, BytecodeData* bytecode)
{
    interpreter_start(context, bytecode);

    // Main loop
    while (!interpreter_finished(context))
        interpreter_step(context);

    return SUCCESS;
}

void interpret_range(EvalContext* context, Branch* branch, int start, int end)
{
    context->preserveLocals = true;

    interpreter_start(context, branch);

    get_frame(context, 0)->pc = start;

    while (!interpreter_finished(context)) {
        interpreter_step(context);

        // Exit if we have reached 'end'
        if (get_frame(context, 0)->branch == branch && get_frame(context, 0)->pc >= end) {
            interpreter_halt(context);
            return;
        }
    }
}

void copy_locals_to_terms(EvalContext* context, Branch* branch)
{
    // Copy locals back to the original terms. Many tests depend on this functionality.
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (is_value(term)) continue;
        TaggedValue* val = get_output(context, term);
        if (val != NULL)
            copy(val, branch->get(i));
    }
}

TaggedValue* get_state_input(EvalContext* cxt)
{
    Term* term = get_pc_term(cxt);
    Dict* currentScopeState = &get_frame(cxt, 0)->state;
    return currentScopeState->insert(get_unique_name(term));
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


} // namespace circa
