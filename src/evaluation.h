// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// evaluation.h
//
// Functions for driving the interpreted evaluator
//

#pragma once

#include "common_headers.h"
#include "function.h"
#include "for_loop.h"
#include "tagged_value.h"
#include "term_list.h"
#include "types/list.h"
#include "types/dict.h"

namespace circa {

struct Frame {
    int pc;
    List locals;
    TaggedValue temporary;
    Dict state;
    Branch* branch;

    typedef bool (*FinishBranchFunc)(EvalContext* context, int flags);

    FinishBranchFunc finishBranch;
};

struct EvalContext
{
    // If this flag is on, then we'll copy all local variables back to their
    // associated terms when we're finished with a stack frame. This is helpful
    // for testing.
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

    // Local variables.
    List stack;

    // Current stack of in-progress terms. Used for introspection.
    TermList callStack;

    List stateStack;

    // List of stack frames
    Frame* frames;
    int numFrames;

    EvalContext()
      : preserveLocals(false),
        interruptSubroutine(false),
        errorOccurred(false),
        frames(NULL),
        numFrames(0)
    {}
    ~EvalContext();
};

void evaluate_branch_internal(EvalContext* context, Branch* branch);
void evaluate_branch_internal(EvalContext* context, Branch* branch, TaggedValue* output);

void evaluate_branch_internal_with_state(EvalContext* context, Term* term,
        Branch* branch);

void evaluate_branch(EvalContext* context, Branch* branch);

// Top-level call. Evalaute the branch and then preserve stack outputs back to terms.
void evaluate_save_locals(EvalContext* context, Branch* branch);

// Shorthand to call evaluate_save_locals with a new EvalContext:
void evaluate_save_locals(Branch* branch);

// Parse input and immediately evaluate it
void evaluate(EvalContext* context, Branch* branch, std::string const& input);
void evaluate(Branch* branch, Term* function, List* inputs);
void evaluate(Term* function, List* inputs);

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

void clear_error(EvalContext* cxt);

std::string context_get_error_message(EvalContext* cxt);

void evaluate_minimum(EvalContext* context, Term* term, TaggedValue* result);
void evaluate_range(EvalContext* context, Branch* branch, int start, int end);

} // namespace circa
