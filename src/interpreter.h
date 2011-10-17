// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {


Frame* push_frame(EvalContext* context, BytecodeData* bytecode);
Frame* push_frame(EvalContext* context, Branch* branch);
void pop_frame(EvalContext* context);
Frame* top_frame(EvalContext* context);
Frame* get_frame(EvalContext* context, int frame);

TaggedValue* get_input(EvalContext* context, Term* term, int index);
TaggedValue* get_input_rel(EvalContext* context, Term* term, int frameDistance, int index);
void consume_input(EvalContext* context, Term* term, int index, TaggedValue* output);
TaggedValue* get_output(EvalContext* context, Term* term);
TaggedValue* get_output_rel(EvalContext* context, Term* term, int frameDistance);
TaggedValue* get_current_output(EvalContext* context);
TaggedValue* get_extra_output(EvalContext* context, Term* term, int index);
TaggedValue* get_extra_output_rel(EvalContext* context, Term* term, int frameDistance, int index);
TaggedValue* get_output_safe(EvalContext* context, Term* term);
TaggedValue* get_local(EvalContext* context, int relativeFrame, int index);

Term* get_caller(OpCall* call);
TaggedValue* get_arg(EvalContext* context, OpCall* call, int index);
int count_args(OpCall* call);

Term* get_term_from_local(EvalContext* context, int local);

// Low-level interpreter control
void interpreter_start(EvalContext* context, BytecodeData* bytecode);
bool interpreter_finished(EvalContext* context);
void interpret(EvalContext* context);
Operation* interpreter_get_next_operation(EvalContext* context);
Branch* interpreter_get_current_branch(EvalContext* context);

// High-level interpreter control
void interpret(EvalContext* context, BytecodeData* bytecode);
void interpret(EvalContext* context, Branch* branch);

void interpret_range(EvalContext* context, Branch* branch, int start, int end);
void interpret_save_locals(EvalContext* context, Branch* branch);
void interpret_save_locals(Branch* branch);
void interpret_minimum(EvalContext* context, Term* term, TaggedValue* result);

void copy_locals_to_terms(EvalContext* context, Branch* branch);
TaggedValue* get_state_input(EvalContext* cxt);

void error_occurred(EvalContext* context, Term* errorTerm, std::string const& message);

} // namespace circa
