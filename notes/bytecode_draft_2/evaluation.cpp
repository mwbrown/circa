// Evaluate only a range of terms, beginning at the term at index 'start', and ending at
// (but not including) the term at index 'end'.
void evaluate_range(EvalContext* context, Branch* branch, int start, int end);

// Evaluate 'term' and every term that it depends on. Will only reevaluate terms
// in the current branch.
void evaluate_minimum(EvalContext* context, Term* term, TaggedValue* result);

// Get the input value (which might be a local or global) for the given term and index.
TaggedValue* get_input(EvalContext* context, Term* term, int index);
TaggedValue* get_input(EvalContext* context, Operation* op, int index);
TaggedValue* get_input(EvalContext* context, OpCall* op, int index);
int get_int_input(EvalContext* context, OpCall* op, int index);

// consume_input will assign 'dest' to the value of the given input. It may copy the
// input value. But, if it's safe to do so, this function will instead swap the value,
// leaving a null behind and preventing the need for a copy.
void consume_input(EvalContext* context, Term* term, int index, TaggedValue* dest);



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

void evaluate_range(EvalContext* context, Branch* branch, int start, int end)
{
    // Genrate bytecode for this range.
    BytecodeWriter bytecode;

    for (int i=start; i < end; i++)
        bc_call(&bytecode, branch->get(i));
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
    push_stack_frame(context, branch);
    push_scope_state(context);
    evaluate_bytecode(context, bytecode.data);

    // Copy locals back to terms
    for (int i=start; i < end; i++) {
        Term* term = branch->get(i);
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


void evaluate_minimum(EvalContext* context, Term* term, TaggedValue* result)
{
    // Short-circuit if the term is a value
    if (is_value(term)) {
        if (result != NULL)
            copy(term, result);
        return;
    }

    Branch* branch = term->owningBranch;

    // Walk upwards, and "mark" every term that this term depends on. Limit this
    // search to the current branch.

    bool *marked = new bool[branch->length()];
    memset(marked, false, sizeof(bool) * branch->length());

    marked[term->index] = true;

    for (int i=term->index; i >= 0; i--) {
        Term* checkTerm = branch->get(i);
        if (checkTerm == NULL)
            continue;

        if (marked[i]) {
            for (int inputIndex=0; inputIndex < checkTerm->numInputs(); inputIndex++) {
                Term* input = checkTerm->input(inputIndex);
                if (input == NULL)
                    continue;
                if (input->owningBranch != branch)
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
            bc_call(&bytecode, branch->get(i));
    }
    bc_finish(&bytecode);

    // Run our bytecode
    push_stack_frame(context, branch);
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

void evaluate(EvalContext* context, Branch* branch, std::string const& input)
{
    int prevHead = branch->length();
    parser::compile(branch, parser::statement_list, input);
    evaluate_range(context, branch, prevHead, branch->length());
}

void evaluate(Branch* branch, Term* function, List* inputs)
{
    EvalContext context;

    TermList inputTerms;
    inputTerms.resize(inputs->length());

    for (int i=0; i < inputs->length(); i++)
        inputTerms.setAt(i, create_value(branch, inputs->get(i)));

    int prevHead = branch->length();
    apply(branch, function, inputTerms);
    evaluate_range(&context, branch, prevHead, branch->length());
}

void evaluate(Term* function, List* inputs)
{
    Branch scratch;
    return evaluate(&scratch, function, inputs);
}

