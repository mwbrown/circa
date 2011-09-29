// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "evaluation.h"
#include "interpreter.h"
#include "introspection.h"
#include "kernel.h"
#include "locals.h"
#include "function.h"
#include "term.h"

namespace circa {

Frame* push_frame(EvalContext* context, Branch* branch)
{
    context->numFrames++;
    context->frames = (Frame*) realloc(context->frames, sizeof(Frame) * context->numFrames);
    Frame* top = &context->frames[context->numFrames-1];
    top->pc = 0;
    top->branch = branch;
    top->locals.initializeNull();
    set_list(&top->locals, branch->length());
    top->state.initializeNull();
    set_dict(&top->state);
    top->finishBranch = NULL;
    return top;
}

void pop_frame(EvalContext* context)
{
    ca_assert(context->numFrames > 0);

    Frame* top = top_frame(context);
    set_null(&top->locals);
    set_null(&top->state);
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

    // Check if the calling function specifies a custom finishBranch handler
    if (frame->finishBranch) {
        bool continueFinish = frame->finishBranch(context, flags);
        if (!continueFinish)
            return;
    }

    if (context->preserveLocals)
        copy_locals_to_terms(context, frame->branch);

    pop_frame(context);
}

bool top_level_finish_branch(EvalContext* context, int flags)
{
    move(&top_frame(context)->state, &context->state);
    return true;
}

void interpreter_start(EvalContext* context, Branch* branch)
{
    Frame* firstFrame = push_frame(context, branch);
    if (is_dict(&context->state))
        copy(&context->state, &firstFrame->state);
    firstFrame->finishBranch = top_level_finish_branch;
}

void interpreter_step(EvalContext* context)
{
    Frame* frame = top_frame(context);

    // Check to finish this branch
    if (frame->pc >= frame->branch->length()) {

        finish_branch(context, 0);
        return;
    }

    Term* term = frame->branch->get(frame->pc);

    switch (term->instruction) {
    case ISN_CALL: {
        TaggedValue* value_pointers[MAX_INPUTS];

        // Copy pointers to value_pointers
        // Input 0 is used for output
        value_pointers[0] = frame->locals[frame->pc];

        int numInputs = term->numInputs();

        for (int i=0; i < numInputs; i++) {

            Term* input = term->input(i);

            if (input == NULL) {
                value_pointers[i+1] = NULL;
            } else if (is_value(input)) {
                value_pointers[i+1] = input;
            } else {
                int relativeFrame = get_frame_distance(input->owningBranch, term);
                Frame* frame = get_frame(context, relativeFrame);
                value_pointers[i+1] = frame->locals[term->input(i)->index];
            }
        }
    
        // Execute the call
        term->evaluateFunc(context, numInputs+1, value_pointers);

        frame->pc += 1;
        break;
    }

    case ISN_CALL_MANUAL:
        get_function_attrs(term->function)->evaluateManual(context);
        break;
    
    case ISN_SKIP:
        frame->pc += 1;
        break;

    default: {
        internal_error("unrecognized term->instruction");
    }
    }
}

bool interpreter_finished(EvalContext* context)
{
    return context->numFrames <= 0;
}

InterpretResult interpret(EvalContext* context, Branch* branch)
{
    interpreter_start(context, branch);

    // Main loop
    while (true) {

        if (interpreter_finished(context))
            return SUCCESS;
        
        interpreter_step(context);
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

} // namespace circa
