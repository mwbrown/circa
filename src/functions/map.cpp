// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

namespace circa {
namespace map_function {

    CA_FUNCTION(evaluate_map)
    {
        EvalContext* context = CONTEXT;
        EvaluateFunc func = get_function_attrs(as_ref(INPUT(0)))->evaluate;

        TaggedValue* list = INPUT(1);
        int count = list->numElements();

        List* output = set_list(OUTPUT, count);

        TaggedValue* pointers[2];
        for (int i=0; i < count; i++) {
            pointers[0] = output->get(i);
            pointers[1] = list->getIndex(i);
            func(context, 3, pointers);
        }
    }

    CA_FUNCTION(evaluate_zip)
    {
        EvalContext* context = CONTEXT;
        EvaluateFunc func = get_function_attrs(as_ref(INPUT(0)))->evaluate;

        TaggedValue* left = INPUT(1);
        TaggedValue* right = INPUT(2);
        int count = left->numElements();

        List* output = set_list(OUTPUT, count);

        TaggedValue* pointers[3];
        for (int i=0; i < count; i++) {
            pointers[0] = output->get(i);
            pointers[1] = left->getIndex(i);
            pointers[2] = right->getIndex(i);
            func(context, 3, pointers);
        }
    }

    CA_FUNCTION(evaluate_zip_vs)
    {
        EvalContext* context = CONTEXT;
        EvaluateFunc func = get_function_attrs(as_ref(INPUT(0)))->evaluate;

        TaggedValue* left = INPUT(1);
        TaggedValue* right = INPUT(2);
        int count = left->numElements();

        List* output = set_list(OUTPUT, count);

        TaggedValue* pointers[3];
        pointers[2] = right;

        for (int i=0; i < count; i++) {
            pointers[0] = output->get(i);
            pointers[1] = right->getIndex(i);
            func(context, 3, pointers);
        }
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate_map, "map(Function,List)->List");
        import_function(kernel, evaluate_zip, "zip(Function,List,List)->List");
        import_function(kernel, evaluate_zip_vs, "zip_vs(Function,List,any)->List");
    }
}
}
