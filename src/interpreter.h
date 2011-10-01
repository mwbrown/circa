// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

typedef char Instruction;

const Instruction ISN_CALL = 1;
const Instruction ISN_CALL_MANUAL = 2;
const Instruction ISN_OPEN_BRANCH = 3;
const Instruction ISN_SKIP = 4;

const int MAX_INPUTS = 100;

Frame* push_frame(EvalContext* context, BytecodeData* bytecode);
Frame* push_frame(EvalContext* context, Branch* branch);
void pop_frame(EvalContext* context);
Frame* top_frame(EvalContext* context);
Frame* get_frame(EvalContext* context, int frame);
Term* get_pc_term(EvalContext* context);

TaggedValue* get_input(EvalContext* context, Term* term, int index);
TaggedValue* get_input_rel(EvalContext* context, Term* term, int frameDistance, int index);
void consume_input(EvalContext* context, Term* term, int index, TaggedValue* output);
TaggedValue* get_current_input(EvalContext* context, int index);
TaggedValue* get_output(EvalContext* context, Term* term);
TaggedValue* get_output_rel(EvalContext* context, Term* term, int frameDistance);
TaggedValue* get_current_output(EvalContext* context);
TaggedValue* get_extra_output(EvalContext* context, Term* term, int index);
TaggedValue* get_extra_output_rel(EvalContext* context, Term* term, int frameDistance, int index);
TaggedValue* get_output_safe(EvalContext* context, Term* term);
TaggedValue* get_local(EvalContext* context, int relativeFrame, int index);

void finish_branch(EvalContext* context, int flags);

// High-level interpreter control
void interpret(EvalContext* context, BytecodeData* bytecode);
void interpret(EvalContext* context, Branch* branch);
//void interpret_range(EvalContext* context, Branch* branch, int start, int end);

// Lower-level interpreter control
void interpreter_start(EvalContext* context, Branch* branch, BytecodeData* bytecode);
void interpreter_start(EvalContext* context, Branch* branch);
void interpreter_step(EvalContext* context);
bool interpreter_finished(EvalContext* context);
void interpreter_halt(EvalContext* context);

void interpret_range(EvalContext* context, Branch* branch, int start, int end);

void copy_locals_to_terms(EvalContext* context, Branch* branch);
TaggedValue* get_state_input(EvalContext* cxt);

void error_occurred(EvalContext* context, Term* errorTerm, std::string const& message);

} // namespace circa
