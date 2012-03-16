// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace list_methods_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(append, "List.append(self :implied_rebind, any) -> List")
    {
        CONSUME_INPUT(0, OUTPUT);
        List* result = List::checkCast(OUTPUT);
        CONSUME_INPUT(1, result->append());
    }
    Type* append_specializeType(Term* term)
    {
        Term* listInput = term->input(0);
        switch (list_get_parameter_type(&listInput->type->parameter)) {
        case LIST_UNTYPED:
            return listInput->type;
        case LIST_TYPED_UNSIZED:
        {
            Type* listElementType = list_get_repeated_type_from_type(listInput->type);
            Type* commonType = find_common_type(listElementType, term->input(1)->type);
            if (commonType == listElementType)
                return listInput->type;
            else
                return create_typed_unsized_list_type(commonType);
        }
        case LIST_TYPED_SIZED:
        case LIST_TYPED_SIZED_NAMED:
        {    
            List elementTypes;
            copy(list_get_type_list_from_type(listInput->type), &elementTypes);
            set_type(elementTypes.append(), term->input(1)->type);
            return create_typed_unsized_list_type(find_common_type(&elementTypes));
        }
        case LIST_INVALID_PARAMETER:
        default:
            return &ANY_T;
        }
    }

    CA_DEFINE_FUNCTION(extend, "List.extend(self :implied_rebind, List) -> List")
    {
        CONSUME_INPUT(0, OUTPUT);
        List* result = List::checkCast(OUTPUT);

        List* additions = List::checkCast(INPUT(1));

        int oldLength = result->length();
        int additionsLength = additions->length();

        result->resize(oldLength + additionsLength);
        for (int i = 0; i < additionsLength; i++)
            copy(additions->get(i), result->get(oldLength + i));
    }

    CA_DEFINE_FUNCTION(count, "List.count(self) -> int")
    {
        List* list = List::checkCast(INPUT(0));
        set_int(OUTPUT, list->length());
    }
    CA_DEFINE_FUNCTION(length, "List.length(self) -> int")
    {
        List* list = List::checkCast(INPUT(0));
        set_int(OUTPUT, list->length());
    }

    CA_DEFINE_FUNCTION(insert, "List.insert(self :implied_rebind, int, any) -> List")
    {
        caValue result;
        CONSUME_INPUT(0, &result);
        caValue* newItem = list_insert(&result, INT_INPUT(1));
        CONSUME_INPUT(2, newItem);
        swap(&result, OUTPUT);
    }

    CA_DEFINE_FUNCTION(slice, "List.slice(self, int start, int fin) -> List")
    {
        List* input = List::checkCast(INPUT(0));
        int start = INT_INPUT(1);
        int end = INT_INPUT(2);
        int length = end - start;
        List* result = List::cast(OUTPUT, length);

        for (int i=0; i < length; i++)
            copy(input->get(start + i), result->get(i));
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);

        LIST_APPEND_FUNC = kernel->get("List.append");
        as_function(LIST_APPEND_FUNC)->specializeType = append_specializeType;
    }

} // namespace list_methods_function
} // namespace circa
