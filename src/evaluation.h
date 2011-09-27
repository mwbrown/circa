// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// evaluation.h
//
// Functions for driving the interpreted evaluator
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
    // If this flag is on, then we'll copy all local variables back to their
    // associated terms when we're finished with a stack frame. This is very
    // helpful for testing and local evaluation.
    bool preserveLocals;

    bool interruptSubroutine;

    TaggedValue subroutineOutput;

    // Error information:
    bool errorOccurred;
    Term* errorTerm;
    TaggedValue errorValue;

    // Tree of persistent state
    TaggedValue state;

    // State used for the current for loop
    ForLoopContext forLoopContext;

    // Intra-program messages
    Dict messages;

    // List of input values, passed in to the script from the caller.
    List argumentList;

    struct Frame {
        List locals;
        Branch* branch;
    };

    // Local variables.
    List stack;

    // Current stack of in-progress terms. Used for introspection.
    TermList callStack;

    List stateStack;

    EvalContext()
      : preserveLocals(false),
        interruptSubroutine(false),
        errorOccurred(false) {}
};

void evaluate_branch_internal(EvalContext* context, Branch* branch);
void evaluate_branch_internal(EvalContext* context, Branch* branch, TaggedValue* output);

void evaluate_branch_internal_with_state(EvalContext* context, Term* term,
        Branch* branch);

void evaluate_branch(EvalContext* context, Branch* branch);

// Top-level call. Evalaute the branch and then preserve stack outputs back to terms.
void evaluate_save_locals(EvalContext* context, Branch* branch);

void copy_locals_to_terms(EvalContext* context, Branch* branch);

// Shorthand to call evaluate_save_locals with a new EvalContext:
void evaluate_save_locals(Branch* branch);

// Evaluate only a range of terms, beginning at the term at index 'start', and ending at
// (but not including) the term at index 'end'.
void evaluate_range(EvalContext* context, Branch* branch, int start, int end);

// Evaluate 'term' and every term that it depends on. Will only reevaluate terms
// in the current branch.
void evaluate_minimum(EvalContext* context, Term* term, TaggedValue* result);

void evaluate_single_term_with_bytecode(EvalContext* context, Term* term);

// Parse input and immediately evaluate it
void evaluate(EvalContext* context, Branch* branch, std::string const& input);
void evaluate(Branch* branch, Term* function, List* inputs);
void evaluate(Term* function, List* inputs);

// Get the input value (which might be a local or global) for the given term and index.
TaggedValue* get_input(EvalContext* context, Term* term, int index);
TaggedValue* get_input(EvalContext* context, Operation* op, int index);
TaggedValue* get_input(EvalContext* context, OpCall* op, int index);
int get_int_input(EvalContext* context, OpCall* op, int index);

// consume_input will assign 'dest' to the value of the given input. It may copy the
// input value. But, if it's safe to do so, this function will instead swap the value,
// leaving a null behind and preventing the need for a copy.
void consume_input(EvalContext* context, Term* term, int index, TaggedValue* dest);

TaggedValue* get_output(EvalContext* context, Term* term);
TaggedValue* get_extra_output(EvalContext* context, Term* term, int index);
TaggedValue* get_state_input(EvalContext* cxt, Term* term);

TaggedValue* get_local(EvalContext* cxt, int relativeFrame, Term* term);
TaggedValue* get_local(EvalContext* cxt, int relativeFrame, int index);

void error_occurred(EvalContext* context, Term* errorTerm, std::string const& message);

void print_runtime_error_formatted(EvalContext& context, std::ostream& output);

Dict* get_current_scope_state(EvalContext* cxt);
Dict* get_scope_state(EvalContext* cxt, int frame);
void push_scope_state(EvalContext* cxt);
void push_scope_state_for_term(EvalContext* cxt, Term* term);
void pop_scope_state(EvalContext* cxt);
void fetch_state_container(Term* term, TaggedValue* container, TaggedValue* output);
void consume_scope_state_field(Term* term, Dict* scopeState, TaggedValue* output);
void save_and_pop_scope_state(EvalContext* cxt, Term* term);

// Saves the state result inside 'result' into the given container, according to
// the unique name of 'term'. This call will consume the value inside 'result',
// so 'result' will be null after this call.
void save_and_consume_state(Term* term, TaggedValue* container, TaggedValue* result);

// Returns whether evaluation has been interrupted, such as with a 'return' or
// 'break' statement, or a runtime error.
bool evaluation_interrupted(EvalContext* context);

void push_stack_frame(EvalContext* context, int size);
void push_stack_frame(EvalContext* context, Branch* branch);
void push_stack_frame(EvalContext* context, List* frame);
void pop_stack_frame(EvalContext* context);
List* get_stack_frame(EvalContext* context, int relativeFrame);

void clear_error(EvalContext* cxt);

std::string context_get_error_message(EvalContext* cxt);

} // namespace circa
