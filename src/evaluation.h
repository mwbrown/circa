// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// evaluation.h
//
// Functions for driving the interpreted evaluator
//
// 

#pragma once

#include "common_headers.h"
#include "for_loop.h"
#include "tagged_value.h"
#include "term_list.h"
#include "types/list.h"
#include "types/dict.h"

namespace circa {

struct EvalContext
{
    bool interruptSubroutine;

    TaggedValue subroutineOutput;

    // Error information:
    bool errorOccurred;
    Term* errorTerm;
    TaggedValue errorValue;

    // Tree of persistent state
    TaggedValue state;

    // State that should be used for the current branch. This is a temporary value
    // only used during evaluation.
    TaggedValue currentScopeState;

    // State used for the current for loop
    ForLoopContext forLoopContext;

    // Intra-program messages
    Dict messages;

    // Stack of input values, used for inputs to functions & blocks.
    List inputStack;

    // Local variables.
    List stack;

    // Current stack of in-progress terms. Used for introspection.
    TermList callStack;

    EvalContext() : interruptSubroutine(false), errorOccurred(false) {}
};

// Evaluate a single term. This is not usually called directly, it's called
// by the interpreter.
void evaluate_single_term(EvalContext* context, Term* term);

void evaluate_branch_internal(EvalContext* context, Branch& branch);
void evaluate_branch_internal(EvalContext* context, Branch& branch, TaggedValue* output);

void evaluate_branch_internal_with_state(EvalContext* context, Term* term,
        Branch& branch);

void evaluate_branch_no_preserve_locals(EvalContext* context, Branch& branch);

// Top-level call. Evalaute the branch and then preserve stack outputs back to terms.
void evaluate_save_locals(EvalContext* context, Branch& branch);
void copy_locals_to_terms(EvalContext* context, Branch& branch);

// Shorthand to call evaluate_save_locals with a new EvalContext:
void evaluate_save_locals(Branch& branch);

// Evaluate only the terms between 'start' and 'end'.
void evaluate_range(EvalContext* context, Branch& branch, int start, int end);

// Evaluate 'term' and every term that it depends on.
void evaluate_minimum(EvalContext* context, Term* term, TaggedValue* result);
void evaluate_minimum_preserve_locals(EvalContext* context, Term* term, TaggedValue* result);

// Parse input and immediately evaluate it
void evaluate(EvalContext* context, Branch& branch, std::string const& input);
void evaluate(Branch& branch, Term* function, List* inputs);
void evaluate(Term* function, List* inputs);

// Get the input value for the given term and index.
TaggedValue* get_input(EvalContext* context, Term* term, int index);

// consume_input will assign 'dest' to the value of the given input. It may copy the
// input value. But, if it's safe to do so, this function will instead swap the value,
// leaving a null behind and preventing the need for a copy.
void consume_input(EvalContext* context, Term* term, int index, TaggedValue* dest);

TaggedValue* get_output(EvalContext* context, Term* term, int outputIndex);
TaggedValue* get_extra_output(EvalContext* context, Term* term, int index);
TaggedValue* get_state_input(EvalContext* cxt, Term* term);

TaggedValue* get_local(EvalContext* cxt, int relativeFrame, Term* term, int outputIndex);
TaggedValue* get_local(EvalContext* cxt, int relativeFrame, int index);

void error_occurred(EvalContext* context, Term* errorTerm, std::string const& message);

void print_runtime_error_formatted(EvalContext& context, std::ostream& output);

Dict* get_current_scope_state(EvalContext* cxt);
void fetch_state_container(Term* term, TaggedValue* container, TaggedValue* output);

// Saves the state result inside 'result' into the given container, according to
// the unique name of 'term'. This call will consume the value inside 'result',
// so 'result' will be null after this call.
void save_and_consume_state(Term* term, TaggedValue* container, TaggedValue* result);

// Returns whether evaluation has been interrupted, such as with a 'return' or
// 'break' statement, or a runtime error.
bool evaluation_interrupted(EvalContext* context);

void push_stack_frame(EvalContext* context, int size);
void push_stack_frame(EvalContext* context, Branch* branch);
void pop_stack_frame(EvalContext* context);
List* get_stack_frame(EvalContext* context, int relativeFrame);

void clear_error(EvalContext* cxt);

std::string context_get_error_message(EvalContext* cxt);

} // namespace circa
