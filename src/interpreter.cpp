// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "bytecode.h"
#include "evaluation.h"
#include "interpreter.h"
#include "introspection.h"
#include "list_shared.h"
#include "kernel.h"
#include "function.h"
#include "term.h"
#include "type.h"

namespace circa {

Frame* push_frame(EvalContext* context, BytecodeData* bytecode)
{
    context->numFrames++;
    context->frames = (Frame*) realloc(context->frames, sizeof(Frame) * context->numFrames);
    Frame* top = &context->frames[context->numFrames-1];
    top->pc = 0;

    top->locals.initializeNull();
    set_list(&top->locals, bytecode->localsCount);

    top->temporaries.initializeNull();
    set_list(&top->temporaries, 0);
    
    top->state.initializeNull();
    set_dict(&top->state);

    top->bytecode = bytecode;
    top->branch = bytecode->branch;
    return top;
}
Frame* push_frame(EvalContext* context, Branch* branch)
{
    return push_frame(context, branch->bytecode);
}

void pop_frame(EvalContext* context)
{
    ca_assert(context->numFrames > 0);

    Frame* top = top_frame(context);
    set_null(&top->locals);
    set_null(&top->temporaries);
    set_null(&top->state);
    context->numFrames--;
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
TaggedValue* get_output(EvalContext* context, Term* term)
{
    Frame* frame = top_frame(context);
    return frame->locals[term->index];
}
TaggedValue* get_output_rel(EvalContext* context, Term* term, int frameDistance)
{
    Frame* frame = get_frame(context, frameDistance);
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

#if 0
TaggedValue* get_arg(EvalContext* context, OpCall* call, int index)
{
    Operation* op = &call->args[index];
    switch (op->type) {
        case OP_INPUT_GLOBAL: {
            OpInputGlobal* gop = (OpInputGlobal*) op;
            return gop->value;
        }

        case OP_OUTPUT_LOCAL:
        case OP_INPUT_LOCAL: {
            OpLocal* lop = (OpLocal*) op;
            Frame* frame = get_frame(context, lop->relativeFrame);
            return list_get_index(&frame->locals, lop->local);
        }

        case OP_INPUT_NULL:
            return NULL;

        case OP_INPUT_INT:
            internal_error("can't read int_input using get_arg");
            break;
    }
    return NULL;
}
#endif

Term* get_term_from_local(EvalContext* context, int local)
{
    Branch* branch = top_frame(context)->branch;
    for (int i=0; i < branch->length(); i++)
        if (branch->get(i)->local == local)
            return branch->get(i);
    return NULL;
}

TaggedValue* follow_input_instruction(EvalContext* context, Operation* op)
{
    switch (op->type) {
        case OP_INPUT_GLOBAL: {
            OpInputGlobal* gop = (OpInputGlobal*) op;
            return gop->value;
        }
        case OP_INPUT_LOCAL: {
            OpLocal* lop = (OpLocal*) op;
            Frame* frame = get_frame(context, lop->relativeFrame);
            return list_get_index(&frame->locals, lop->local);
        }
        case OP_INPUT_NULL:
            return NULL;
    }
    return NULL;
}

void consume_input_instruction(EvalContext* context, Operation* op, TaggedValue* output)
{
    switch (op->type) {
        case OP_INPUT_INT: {
            set_int(output, ((OpInputInt*) op)->value);
            break;
        default:
            copy(follow_input_instruction(context, op), output);
        }
    }
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

void interpreter_start(EvalContext* context, BytecodeData* bytecode)
{
    push_frame(context, bytecode);
}

bool interpreter_finished(EvalContext* context)
{
    return context->numFrames == 0;
}

Operation* interpreter_get_next_operation(EvalContext* context)
{
    Frame* frame = top_frame(context);
    return &frame->bytecode->operations[frame->pc];
}

Branch* interpreter_get_current_branch(EvalContext* context)
{
    return top_frame(context)->bytecode->branch;
}

void interpret(EvalContext* context)
{
    List argumentStack;
    argumentStack.resize(CA_MAX_INPUTS);

    // Main loop.
    while (true) {

        Frame* frame = top_frame(context);
        BytecodeData* bytecode = frame->bytecode;

        Operation* op = &bytecode->operations[frame->pc];

        switch (op->type) {
        case OP_INPUT_LOCAL:
        case OP_INPUT_GLOBAL:
        case OP_INPUT_NULL:
        case OP_INPUT_INT:
        case OP_OUTPUT_LOCAL:
            // ignore these ops
            frame->pc += 1;
            continue;

        case OP_CALL: {
#if 0

            OpCall* cop = (OpCall*) op;

            //std::cout << "calling: "; dump_call(context, cop);
            
            #if CIRCA_THROW_ON_ERROR
            try {
            #endif

            get_function_attrs(cop->func)->evaluate(context, cop);
            frame->pc = frame->pc + 1;

            #if CIRCA_THROW_ON_ERROR
            } catch (std::exception const& e) { error_occurred(context, cop->term, e.what()); }
            #endif

#endif
            continue;
        }


        case OP_STOP:
            while (!interpreter_finished(context))
                pop_frame(context);
            return;

        case OP_PAUSE:
            frame->pc += 1;
            return;

        case OP_PAUSE_IF_ERROR:
            frame->pc += 1;
            if (context->errorOccurred)
                return;
            continue;

        case OP_JUMP: {
            OpJump* jop = (OpJump*) op;
            frame->pc += jop->offset;
            continue;
        }

#if 0 // FIXME
        case OP_JUMP_IF: {
            OpJump* jop = (OpJump*) op;
            TaggedValue* input = get_arg(context, (OpCall*) op, 0);
            ca_assert(is_bool(input));
            if (as_bool(input)) {
                ca_assert(jop->offset != 0);
                frame->pc += jop->offset;
            } else {
                frame->pc += 1;
            }
            continue;
        }
        case OP_JUMP_IF_NOT: {
            OpJump* jop = (OpJump*) op;
            TaggedValue* input = get_arg(context, (OpCall*) op, 0);
            ca_assert(is_bool(input));
            if (!as_bool(input)) {
                ca_assert(jop->offset != 0);
                frame->pc += jop->offset;
            } else {
                frame->pc += 1;
            }
            continue;
        }
        case OP_JUMP_IF_NOT_EQUAL: {
            OpJump* jop = (OpJump*) op;
            TaggedValue* left = get_arg(context, (OpCall*) op, 0);
            TaggedValue* right = get_arg(context, (OpCall*) op, 1);
            if (!equals(left,right)) {
                ca_assert(jop->offset != 0);
                frame->pc += jop->offset;
            } else {
                frame->pc += 1;
            }
            continue;
        }
        case OP_JUMP_IF_LESS_THAN: {
            OpJump* jop = (OpJump*) op;
            TaggedValue* left = get_arg(context, (OpCall*) op, 0);
            TaggedValue* right = get_arg(context, (OpCall*) op, 1);
            if (to_float(left) < to_float(right)) {
                ca_assert(jop->offset != 0);
                frame->pc += jop->offset;
            } else {
                frame->pc += 3;
            }
            continue;
        }

        case OP_PUSH_FRAME: {
            OpPushBranch* bop = (OpPushBranch*) op;

            Branch* branch = nested_contents(bop->term);
            update_bytecode_for_branch(branch);
            bytecode = branch->bytecode;

            List locals;
            locals.resize(bytecode->localsCount);

            int count = count_args((OpCall*) bop);

            for (int arg=0; arg < count; arg++)
                consume_arg(context, (OpCall*) bop, arg, locals[arg]);

            push_frame(context, bytecode);
            Frame* frame = top_frame(context);

            swap(&frame->locals, &locals);
            frame->pc = 0;
            continue;
        }

        case OP_POP_FRAME: {

            Frame* parent = get_frame(context, 1);

            // In the parent frame, find the start of the output instructions.
            int parentPc = parent->pc + 1;
            for (; ; parentPc++) {
                switch (parent->bytecode->operations[parentPc].type) {
                    case OP_INPUT_GLOBAL:
                    case OP_INPUT_LOCAL:
                    case OP_INPUT_NULL:
                        continue;
                }
                break;
            }

            // Iterate through input instructions, and copy outputs to the
            // upper frame.
            int lookahead;
            for (lookahead=0; ; lookahead++) {
                Operation* inputOp = op + 1 + lookahead;

                Operation* outputOp = &parent->bytecode->operations[parentPc + lookahead];

                if (outputOp->type != OP_OUTPUT_LOCAL)
                    break;

                TaggedValue* dest = parent->locals[((OpLocal*)outputOp)->local];

                switch (inputOp->type) {
                    case OP_INPUT_GLOBAL: {
                        OpInputGlobal* gop = (OpInputGlobal*) inputOp;
                        copy(gop->value, dest);
                        continue;
                    }
                    case OP_INPUT_LOCAL: {
                        OpLocal* lop = (OpLocal*) inputOp;
                        Frame* inputFrame = get_frame(context, lop->relativeFrame);
                        copy(inputFrame->locals[lop->local], dest);
                        continue;
                    }
                    case OP_INPUT_NULL:
                        set_null(dest);
                        continue;
                }
                break;
            }

            pop_frame(context);

            if (context->numFrames == 0)
                return;

            Frame* frame = top_frame(context);
            frame->pc += 1 + lookahead;
            bytecode = frame->bytecode;
            continue;
        }

        case OP_ASSIGN_LOCAL: {
            TaggedValue* local = top_frame(context)->locals[((OpAssignLocal*) op)->local];
            consume_input_instruction(context, op + 1, local);
            frame->pc += 2;
            continue;
        }
#endif

        default:
            internal_error("in evaluate_bytecode, unrecognized op type");
        }
    }
}

void interpret(EvalContext* context, Branch* branch)
{
    update_bytecode_for_branch(branch);
    interpreter_start(context, branch->bytecode);
    interpret(context);
}

void interpret(EvalContext* context, BytecodeData* bytecode)
{
    interpreter_start(context, bytecode);
    interpret(context);
}

void interpret_range(EvalContext* context, Branch* branch, int start, int end)
{
    BytecodeWriter writer;
    bc_start_branch(&writer, branch);

    for (int i=start; i < end; i++)
        bc_call(&writer, branch->get(i));

    bc_pause(&writer);

    // Create a new branch if needed, otherwise reuse the existing one.
    if (context->numFrames == 0)
        interpreter_start(context, writer.data);
    else {
        Frame* topFrame = top_frame(context);
        topFrame->bytecode = writer.data;
        topFrame->pc = 0;
    }

    interpret(context);

    top_frame(context)->bytecode = NULL;

    //interpret_save_locals(context);
}

void interpret_save_locals(EvalContext* context, Branch* branch)
{
    BytecodeWriter writer;
    bc_start_branch(&writer, branch);

    for (int i=0; i < branch->length(); i++)
        bc_call(&writer, branch->get(i));

    bc_pause(&writer);

    interpreter_start(context, writer.data);
    interpret(context);

    copy_locals_to_terms(context, branch);
}

void interpret_save_locals(Branch* branch)
{
    EvalContext context;
    interpret_save_locals(&context, branch);
}

void interpret_minimum(EvalContext* context, Term* term, TaggedValue* result)
{
    // TODO
}

void copy_locals_to_terms(EvalContext* context, Branch* branch)
{
    Frame* frame = top_frame(context);

    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (is_value(term))
            continue;
        if (term->local == -1)
            continue;
        TaggedValue* val = frame->locals[term->local];
        if (val != NULL)
            copy(val, branch->get(i));
    }
}

TaggedValue* get_state_input(EvalContext* cxt)
{
    internal_error("get_state_input broken");
    return NULL;

#if 0
    Term* term = get_pc_term(cxt);
    Dict* currentScopeState = &get_frame(cxt, 0)->state;
    return currentScopeState->insert(get_unique_name(term));
#endif
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

void dump_call(EvalContext* context, OpCall* op)
{
#if 0 // FIXME
    if (op->term != NULL)
        std::cout << global_id(op->term) << " ";

    std::cout << op->func->name << " ";

    int count = count_args(op);
    for (int i=0; i < count; i++) {
        if (i > 0)
            std::cout << ", ";

        switch (op->args[i].type) {
        case OP_INPUT_GLOBAL:
            std::cout << "global(" << get_arg(context, op, i)->toString() << ")";
            break;
        case OP_INPUT_LOCAL: {
            OpLocal* lop = (OpLocal*) &op->args[i].type;
            std::cout << "local(" << lop->relativeFrame << ":" << lop->local << ")";
            std::cout << "(" << get_arg(context, op, i)->toString() << ")";
            break;
        }
        case OP_INPUT_NULL:
            std::cout << "null(" << get_arg(context, op, i)->toString() << ")";
            break;
        case OP_OUTPUT_LOCAL:
            std::cout << "output(" << get_arg(context, op, i)->toString() << ")";
            break;
        case OP_INPUT_INT:
            std::cout << "int(" << ((OpInputInt*) &op->args[i])->value << ")";
            break;
        }
    }
    std::cout << std::endl;
#endif
}

void apply(Term* function, List* args)
{
    get_function_attrs(function)->evaluate(args);
}

} // namespace circa
