// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "circa.h"
#include "../importing.h"
#include "../importing_macros.h"

#include "types/ref.h"

namespace circa {
namespace vectorize_vv_function {

    Type* specializeType(Term* caller)
    {
        Type* lhsType = caller->input(0)->type;
        if (is_list_based_type(lhsType))
            return lhsType;
        return &LIST_T;
    }

    CA_FUNCTION(evaluate)
    {
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
    }

    void setup(Branch* kernel)
    {
        Term* func = import_function(kernel, evaluate,
                "vectorize_vv(List,List) -> List");
        get_function_attrs(func)->specializeType = specializeType;
    }
}
} // namespace circa
