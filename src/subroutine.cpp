// Copyright (c) Paul Hodge. See LICENSE file for license terms.
 
#include "common_headers.h"

#include "branch.h"
#include "code_iterators.h"
#include "building.h"
#include "bytecode.h"
#include "evaluation.h"
#include "function.h"
#include "importing_macros.h"
#include "interpreter.h"
#include "kernel.h"
#include "refactoring.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "update_cascades.h"
#include "token.h"
#include "term.h"
#include "type.h"

#include "subroutine.h"

namespace circa {

namespace subroutine_f {

    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, "def ", term, token::DEF);

        FunctionAttrs* func = get_function_attrs(term);

        function_format_header_source(source, func);

        if (!is_native_function(func))
            format_branch_source(source, nested_contents(term), term);
    }
}

Term* get_subroutine_input_placeholder(Branch* contents, int index)
{
    return contents->get(index + 1);
}

Type* get_subroutine_output_type(Branch* contents)
{
    return as_type(as_function_attrs(contents->get(0)).outputTypes[0]);
}

CA_FUNCTION(evaluate_subroutine)
{
#if 0
    EvalContext* context = CONTEXT;
    Term* caller = CALLER;
    Term* function = caller->function;
    Branch* contents = nested_contents(function);
    int numInputs = caller->numInputInstructions();

    List stackFrame;
    stackFrame.resize(contents->length());

    // Fetch inputs and copy-cast them to placeholders
    for (int i=0; i < numInputs; i++) {

        TaggedValue* input = get_input(context, caller, i);

        Term* placeholderTerm = get_subroutine_input_placeholder(contents, i);
        TaggedValue* placeholder = stackFrame[placeholderTerm->index];

        if (input == NULL) {
            set_null(placeholder);
            continue;
        }

        Type* inputType = function_get_input_type(function, i);

        bool success = cast(input, inputType, placeholder);
        if (!success) {
            std::stringstream msg;
            msg << "Couldn't cast input " << i << " to " << inputType->name;
            return error_occurred(context, caller, msg.str());
        }
    }

    context->interruptSubroutine = false;
    context->callStack.append(caller);
    push_frame(context, contents);
    set_null(&context->subroutineOutput);

    // Fetch state container
    push_scope_state(context);

    if (is_function_stateful(function))
        fetch_state_container(caller, get_scope_state(context, 1),
                get_scope_state(context, 0));

    // Evaluate contents
    List outputs;

    // Evaluate each term
    evaluate_branch_with_bytecode(context, contents);

    // Fetch output
    Type* outputType = get_subroutine_output_type(contents);

    ca_assert(is_list(&context->subroutineOutput) || is_null(&context->subroutineOutput));

    if (is_list(&context->subroutineOutput))
        swap(&context->subroutineOutput, &outputs);
    
    set_null(&context->subroutineOutput);

    if (context->errorOccurred) {
        outputs.resize(1);
        set_null(outputs.get(0));
    } else if (outputType == &VOID_T) {

        set_null(outputs.get(0));

    } else {

        TaggedValue* output0 = outputs.get(0);

        bool castSuccess = cast(output0, outputType, outputs.get(0));
        
        if (!castSuccess) {
            std::stringstream msg;
            msg << "Couldn't cast output " << output0->toString()
                << " to type " << outputType->name;

            error_occurred(context, caller, msg.str());
        }
    }

    // Clean up
    pop_frame(context);
    context->callStack.pop();
    context->interruptSubroutine = false;
    
    //evaluate_subroutine_internal(context, caller, contents, &inputs, &outputs);

    // Preserve state
    if (is_function_stateful(function))
        save_and_pop_scope_state(context, caller);

    // Write output
    TaggedValue* outputDest = get_output(context, caller);
    if (outputDest != NULL)
        swap(outputs[0], outputDest);

    // Write extra outputs
    for (int i=1; i < outputs.length(); i++)
        swap(outputs[i], get_extra_output(context, caller, i-1));
#endif
}

void subroutine_write_calling_bytecode(BytecodeWriter* writer, Term* term)
{
    bc_push_frame(writer, term->function);

    bool hasState = function_has_inlined_state(term->function);

    // Implicit state input
    if (hasState)
        bc_write_state_arg(writer);

    // Regular inputs
    for (int i=0; i < term->numInputs(); i++)
        bc_write_input(writer, term->owningBranch, term->input(i));

    bc_write_output(writer, term);
}

void subroutine_write_nested_bytecode(BytecodeWriter* writer, Term* function)
{
    ca_assert(is_subroutine(function));
    Branch* contents = nested_contents(function);

    for (int i=0; i < contents->length(); i++) {
        if (contents->get(i)->name == "#attributes")
            continue;
        bc_call(writer, contents->get(i));
    }

    bc_pop_frame(writer);
}

bool is_subroutine(Term* term)
{
    if (term->type != &FUNCTION_T)
        return false;
    if (!has_nested_contents(term))
        return false;
    if (nested_contents(term)->length() < 1)
        return false;
    if (term->contents(0)->type != &FUNCTION_ATTRS_T)
        return false;
    return get_function_attrs(term)->evaluate == evaluate_subroutine;
}

Term* find_enclosing_subroutine(Term* term)
{
    Term* parent = get_parent_term(term);
    if (parent == NULL)
        return NULL;
    if (is_subroutine(parent))
        return parent;
    return find_enclosing_subroutine(parent);
}

int get_input_index_of_placeholder(Term* inputPlaceholder)
{
    ca_assert(inputPlaceholder->function == INPUT_PLACEHOLDER_FUNC);
    return inputPlaceholder->index - 1;
}

void initialize_subroutine(Term* sub)
{
    // Install evaluate function
    get_function_attrs(sub)->evaluate = evaluate_subroutine;
}

void finish_building_subroutine(Term* sub, Term* outputType)
{
    subroutine_check_to_append_implicit_return(sub);
    finish_update_cascade(nested_contents(sub));
}

void subroutine_check_to_append_implicit_return(Term* sub)
{
    // Do nothing if this subroutine already ends with a return
    Branch* contents = nested_contents(sub);
    for (int i=contents->length()-1; i >= 0; i--) {
        Term* term = contents->get(i);
        if (term->function == RETURN_FUNC)
            return;

        // if we found a comment() then keep searching
        if (term->function == COMMENT_FUNC)
            continue;

        // otherwise, break so that we'll insert a return()
        break;
    }

    post_compile_term(apply(contents, RETURN_FUNC, TermList(NULL)));
}

void store_locals(Branch& branch, TaggedValue* storageTv)
{
    touch(storageTv);
    set_list(storageTv);
    List* storage = List::checkCast(storageTv);
    storage->resize(branch.length());
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];

        if (term == NULL) continue;

        if (term->type == &FUNCTION_ATTRS_T)
            continue;

        copy(term, storage->get(i));
    }
}

void restore_locals(TaggedValue* storageTv, Branch& branch)
{
    if (!is_list(storageTv))
        internal_error("storageTv is not a list");

    List* storage = List::checkCast(storageTv);

    // The function branch may be longer than our list of locals. 
    int numItems = storage->length();
    for (int i=0; i < numItems; i++) {
        Term* term = branch[i];

        if (term == NULL) continue;

        if (term->type == &FUNCTION_ATTRS_T)
            continue;

        copy(storage->get(i), term);
    }
}

} // namespace circa
