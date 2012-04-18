// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace list_function {

    CA_START_FUNCTIONS;

    Type* specializeType(Term* term)
    {
        List inputTypes;
        for (int i=0; i < term->numInputs(); i++)
            set_type(inputTypes.append(), term->input(i)->type);

        return as_type(create_tuple_type(&inputTypes));
    }

    CA_DEFINE_FUNCTION(evaluate, "list(any :multiple) -> List")
    {
        // Variadic arg handling will already have turned this into a list
        caValue* out = circa_output(STACK, 0);
        circa_copy(circa_input(STACK, 0), out);
        if (!circa_is_list(out))
            circa_set_list(out, 0);
    }

    CA_DEFINE_FUNCTION(repeat, "repeat(any, int) -> List")
    {
        set_list(OUTPUT);
        List* result = List::checkCast(OUTPUT);
        caValue* source = INPUT(0);
        int repeatCount = INT_INPUT(1);

        result->resize(repeatCount);

        for (int i=0; i < repeatCount; i++)
            copy(source, result->get(i));
    }

    CA_DEFINE_FUNCTION(blank_list, "blank_list(int) -> List")
    {
        set_list(OUTPUT);
        List* result = List::checkCast(OUTPUT);
        result->resize(0);
        result->resize(INT_INPUT(0));
    }

    CA_DEFINE_FUNCTION(resize, "resize(List, int) -> List")
    {
        copy(INPUT(0), OUTPUT);
        List* result = List::checkCast(OUTPUT);
        result->resize(INT_INPUT(1));
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);

        FUNCS.list = kernel->get("list");

        as_function(FUNCS.list)->specializeType = specializeType;
    }
}
}
