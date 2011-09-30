// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../importing.h"
#include "../importing_macros.h"

#include "types/ref.h"

namespace circa {
namespace overloaded_function {

    // update_overloaded_function_call will update the nested_contents of the
    // given term, with an appropriate specialized call.
    void update_overloaded_function_call(Term* term)
    {
        TermList inputs;
        term->inputsToList(inputs);

        Branch* contents = nested_contents(term);
        clear_branch(contents);

        List& overloads = get_function_attrs(term->function)->parameters;

        ca_assert(overloads.length() > 0);

        // Walk through overloads and find the first one that fits
        Term* matchedOverload = NULL;
        for (int i=0; i < overloads.length(); i++) {
            Term* overload = as_ref(overloads[i]);

            if (inputs_statically_fit_function(overload, inputs)) {
                matchedOverload = overload;
                break;
            }
        }

        // If one was found, write a call to nested_contents
        if (matchedOverload != NULL) {
            Term* specialized = apply(contents, matchedOverload, inputs);
            change_declared_type(term, specialized->type);
            return;
        }

        // Otherwise, leave nested_contents blank, we'll do a dynamic overload.
    }

    CA_FUNCTION(evaluate_overload)
    {
        Term* term = CALLER;

        Branch* contents = nested_contents(term);

        // Check if there is a specialized call.
        if (contents->length() > 0) {
            Term* specialized = contents->get(0);
            get_function_attrs(specialized->function)->evaluate(CONTEXT, specialized);
            return;
        }

        // No specialized call, perform a dynamic lookup
        List& overloads = get_function_attrs(term->function)->parameters;

#if 0
        for (int i=0; i < overloads.length(); i++) {
            Term* overload = as_ref(overloads[i]);

            if (values_fit_function_dynamic(overload, inputs)) {
                get_function_attrs(specialized->function)->evaluate(CONTEXT, specialized);
                return;
            }
        }
#endif

        error_occurred(CONTEXT, CALLER, "No matching specialized function");
    }

#if 0
    CA_FUNCTION(evaluate_dynamic_overload)
    {
        Branch* contents = nested_contents(CALLER);

        // FIXME
        Term* call = contents->get(0);
        call->evaluateFunc(CONTEXT, _count, _in);

        Term* func = CALLER->function;
        FunctionAttrs* funcAttrs = get_function_attrs(func);

        List& overloads = get_function_attrs(func)->parameters;

        int numInputs = NUM_INPUTS;

        // Dynamically specialize this function
        Term* specializedFunc = NULL;
        for (int i=0; i < overloads.length(); i++) {
            Term* overload = as_ref(overloads[i]);

            // Fail if wrong # of inputs
            if (!funcAttrs->variableArgs && (function_num_inputs(funcAttrs) != numInputs))
                continue;

            // Check each input
            bool inputsMatch = true;
            for (int i=0; i < numInputs; i++) {
                Type* type = function_get_input_type(overload, i);
                TaggedValue* value = INPUT(i);
                if (value == NULL)
                    continue;
                if (!cast_possible(value, type)) {
                    inputsMatch = false;
                    break;
                }
            }

            if (!inputsMatch)
                continue;

            specializedFunc = overload;
            break;
        }

        if (specializedFunc != NULL) {
            bool alreadyGenerated = (contents->length() > 0)
                && contents->get(0)->function == specializedFunc;
            if (!alreadyGenerated) {
                clear_branch(contents);
                TermList inputs;
                CALLER->inputsToList(inputs);
                apply(contents, specializedFunc, inputs);
                //change_declared_type(CALLER, contents[0]->type);
            }
            TaggedValue output;
            evaluate_branch_internal(CONTEXT, contents, &output);
            cast(&output, contents->get(0)->type, OUTPUT);
        } else {
            std::stringstream msg;
            msg << "specialized func not found for: " << CALLER->function->name;
            return error_occurred(CONTEXT, CALLER, msg.str());
        }
    }
#endif

    Type* overload_specialize_type(Term* term)
    {
        Branch* contents = nested_contents(term);
        return contents->get(0)->type;
    }

    int num_overloads(Term* func)
    {
        return get_function_attrs(func)->parameters.length();
    }

    Term* get_overload(Term* func, int index)
    {
        return get_function_attrs(func)->parameters[index]->asRef();
    }

    Term* find_overload(Term* func, const char* name)
    {
        for (int i=0; i < num_overloads(func); i++) {
            Term* overload = get_overload(func, i);
            if (overload->name == name)
                return overload;
        }
        return NULL;
    }

    void update_function_signature(Term* term)
    {
        List& parameters = get_function_attrs(term)->parameters;

        ca_assert(parameters.length() > 0);
        int argumentCount = function_num_inputs(get_function_attrs(as_ref(parameters[0])));
        bool variableArgs = false;
        List outputTypes;

        for (int i=0; i < parameters.length(); i++) {

            Term* overload = as_ref(parameters[i]);
            FunctionAttrs* overloadAttrs = get_function_attrs(overload);

            if (argumentCount != function_num_inputs(overloadAttrs))
                variableArgs = true;
            if (overloadAttrs->variableArgs)
                variableArgs = true;
            set_type(outputTypes.append(), function_get_output_type(overload, 0));
        }

        Branch* result = nested_contents(term);
        result->shorten(1);
        int placeholderCount = variableArgs ? 1 : argumentCount;
        for (int i=0; i < placeholderCount; i++)
            apply(result, INPUT_PLACEHOLDER_FUNC, TermList());
        Type* outputType = find_common_type(&outputTypes);
        FunctionAttrs* attrs = get_function_attrs(term);
        set_type_list(&attrs->outputTypes, outputType);
        attrs->variableArgs = variableArgs;
    }

    void setup_overloaded_function(Term* term, std::string const& name,
            TermList const& overloads)
    {
        nested_contents(term);
        initialize_function(term);

        FunctionAttrs* attrs = get_function_attrs(term);
        attrs->name = name;
        attrs->evaluate = evaluate_overload;
        attrs->postInputChange = update_overloaded_function_call;
        attrs->createsStackFrame = false;
        // attrs->specializeType = overload_specialize_type;

        List& parameters = get_function_attrs(term)->parameters;
        parameters.clear();
        parameters.resize(overloads.length());

        for (int i=0; i < overloads.length(); i++)
            set_ref(parameters[i], overloads[i]);

        update_function_signature(term);
    }

    Term* create_overloaded_function(Branch* branch, std::string const& name,
        TermList const& overloads)
    {
        Term* result = create_value(branch, &FUNCTION_T, name);
        setup_overloaded_function(result, name, overloads);
        return result;
    }

    void overloaded_func_post_compile(Term* term)
    {
        TermList inputs;
        term->inputsToList(inputs);
        setup_overloaded_function(term, term->name, inputs);
    }

    void append_overload(Term* overloadedFunction, Term* overload)
    {
        List& parameters = get_function_attrs(overloadedFunction)->parameters;
        set_ref(parameters.append(), overload);
        update_function_signature(overloadedFunction);
    }

    CA_FUNCTION(evaluate_declaration)
    {
        // Make our output of type Function so that the type checker doesn't
        // get mad.
        change_type(OUTPUT, unbox_type(FUNCTION_TYPE));
    }

    void setup(Branch* kernel)
    {
        OVERLOADED_FUNCTION_FUNC = import_function(kernel, evaluate_declaration,
                "overloaded_function(Function...) -> Function");
        get_function_attrs(OVERLOADED_FUNCTION_FUNC)->postCompile = overloaded_func_post_compile;
    }
}
}
