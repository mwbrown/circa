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
#if 0
        // Fetch function
        TaggedValue* funcParam = &get_function_attrs(CALLER->function)->parameter;
        Term* funcTerm = as_ref(funcParam);
        EvaluateFunc func = get_function_attrs(funcTerm)->evaluate;

        EvalContext* context = CONTEXT;

        TaggedValue* left = INPUT(0);
        TaggedValue* right = INPUT(1);
        int count = left->numElements();

        List* output = set_list(OUTPUT, count);

        TaggedValue* pointers[3];

        pointers[1] = right;

        for (int i=0; i < count; i++) {
            pointers[0] = left->getIndex(i);
            pointers[2] = output->get(i);
            func(context, 3, pointers);
        }
#endif
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
#if 0
        // Fetch function
        TaggedValue* funcParam = &get_function_attrs(CALLER->function)->parameter;
        Term* funcTerm = as_ref(funcParam);
        EvaluateFunc func = get_function_attrs(funcTerm)->evaluate;

        EvalContext* context = CONTEXT;

        TaggedValue* left = INPUT(0);
        TaggedValue* right = INPUT(1);
        int count = left->numElements();

        List* output = set_list(OUTPUT, count);

        TaggedValue* pointers[3];

        for (int i=0; i < count; i++) {
            pointers[0] = left->getIndex(i);
            pointers[1] = right->getIndex(i);
            pointers[2] = output->get(i);
            func(context, 3, pointers);
        }
#endif
    }

    void setup(Branch* kernel)
    {
        Term* vs = import_function(kernel, evaluate_vs,
                "vectorize_vs(List,any) -> List");
        get_function_attrs(vs)->specializeType = specializeType_vs;

        Term* vv = import_function(kernel, evaluate_vv,
                "vectorize_vv(List,List) -> List");
        get_function_attrs(vv)->specializeType = specializeType_vv;
    }
}
} // namespace circa
