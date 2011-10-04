// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "circa.h"
#include "../importing.h"
#include "../importing_macros.h"

#include "types/ref.h"

namespace circa {
namespace vectorized_functions {

    Type* specializeType_vs(Term* caller)
    {
        #if 0
        Term* lhsType = caller->input(0)->type;
        if (is_list_based_type(unbox_type(lhsType)))
            return lhsType;
        #endif
        return &LIST_T;
    }

    CA_FUNCTION(evaluate_vs)
    {
#if 0 // FIXME
        EvalContext* context = CONTEXT;
        Branch* contents = nested_contents(CALLER);
        TaggedValue input0, input1;
        copy(INPUT(0), &input0);
        copy(INPUT(1), &input1);
        int listLength = input0.numElements();

        Term* input0_placeholder = contents->get(0);
        Term* input1_placeholder = contents->get(1); 
        Term* content_output = contents->get(2); 

        push_stack_frame(CONTEXT, contents);

        // Prepare output
        List output;
        output.resize(listLength);

        // Copy right input once
        swap(&input1, get_local(context, 0, input1_placeholder));

        // Evaluate vectorized call, once for each input
        for (int i=0; i < listLength; i++) {
            // Copy left into placeholder
            swap(input0.getIndex(i), get_local(context, 0, input0_placeholder));

            evaluate_single_term_with_bytecode(CONTEXT, content_output);

            // Save output
            swap(get_local(context, 0, content_output), output[i]);
        }

        pop_stack_frame(CONTEXT);

        swap(&output, OUTPUT);
#endif
    }

    void post_input_change_vs(Term* term)
    {
        // Update generated code
        Branch* contents = nested_contents(term);
        clear_branch(contents);

        TaggedValue* funcParam = &get_function_attrs(term->function)->parameter;
        if (funcParam == NULL || !is_ref(funcParam))
            return;

        Term* func = as_ref(funcParam);
        Term* left = term->input(0);
        Term* right = term->input(1);

        if (func == NULL || left == NULL || right == NULL)
            return;

        Term* leftPlaceholder = apply(contents, INPUT_PLACEHOLDER_FUNC, TermList());
        change_declared_type(leftPlaceholder, infer_type_of_get_index(left));

        Term* rightPlaceholder = apply(contents, INPUT_PLACEHOLDER_FUNC, TermList());
        change_declared_type(rightPlaceholder, right->type);

        apply(contents, func, TermList(leftPlaceholder, rightPlaceholder));
    }

    Type* specializeType_vv(Term* caller)
    {
        Type* lhsType = caller->input(0)->type;
        if (is_list_based_type(lhsType))
            return lhsType;
        return &LIST_T;
    }
    CA_FUNCTION(evaluate_vv)
    {
#if 0 // FIXME
        EvalContext* context = CONTEXT;
        Branch* contents = nested_contents(CALLER);
        TaggedValue input0, input1;

        if (num_elements(INPUT(0)) != num_elements(INPUT(1)))
            return error_occurred(CONTEXT, CALLER, "Input lists have different lengths");

        copy(INPUT(0), &input0);
        copy(INPUT(1), &input1);
        int listLength = input0.numElements();

        Term* input0_placeholder = contents->get(0);
        Term* input1_placeholder = contents->get(1); 
        Term* content_output = contents->get(2); 

        push_stack_frame(context, contents);

        // Prepare output
        List output;
        output.resize(listLength);

        // Evaluate vectorized call, once for each input
        for (int i=0; i < listLength; i++) {
            // Copy inputs into placeholder
            swap(input0.getIndex(i), get_local(context, 0, input0_placeholder));
            swap(input1.getIndex(i), get_local(context, 0, input1_placeholder));

            evaluate_single_term_with_bytecode(CONTEXT, content_output);

            // Save output
            swap(get_local(context, 0, content_output), output[i]);
        }

        pop_stack_frame(context);

        swap(&output, OUTPUT);
#endif
    }
    
    void post_input_change_vv(Term* term)
    {
        // Update generated code
        Branch* contents = nested_contents(term);
        clear_branch(contents);

        TaggedValue* funcParam = &get_function_attrs(term->function)->parameter;
        if (funcParam == NULL || !is_ref(funcParam))
            return;

        Term* func = as_ref(funcParam);
        Term* left = term->input(0);
        Term* right = term->input(1);

        if (func == NULL || left == NULL || right == NULL)
            return;

        Term* leftPlaceholder = apply(contents, INPUT_PLACEHOLDER_FUNC, TermList());
        change_declared_type(leftPlaceholder, infer_type_of_get_index(left));

        Term* rightPlaceholder = apply(contents, INPUT_PLACEHOLDER_FUNC, TermList());
        change_declared_type(rightPlaceholder, infer_type_of_get_index(right));

        apply(contents, func, TermList(leftPlaceholder, rightPlaceholder));
    }

    void setup(Branch* kernel)
    {
        Term* vs = import_function(kernel, evaluate_vs,
                "vectorize_vs(List,any) -> List");
        get_function_attrs(vs)->specializeType = specializeType_vs;
        get_function_attrs(vs)->postInputChange = post_input_change_vs;

        Term* vv = import_function(kernel, evaluate_vv,
                "vectorize_vv(List,List) -> List");
        get_function_attrs(vv)->specializeType = specializeType_vv;
        get_function_attrs(vv)->postInputChange = post_input_change_vv;
    }
}
} // namespace circa