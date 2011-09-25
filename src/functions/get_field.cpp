// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "circa.h"

#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace get_field_function {

    CA_FUNCTION(evaluate)
    {
        TaggedValue* head = INPUT(0);

        for (int nameIndex=1; nameIndex < NUM_INPUTS; nameIndex++) {
            std::string const& name = INPUT(nameIndex)->asString();

            int fieldIndex = list_find_field_index_by_name(head->value_type, name);

            if (fieldIndex == -1)
                return error_occurred(CONTEXT, CALLER, "field not found: " + name);

            head = head->getIndex(fieldIndex);
        }

        copy(head, OUTPUT);
    }

    Type* specializeType(Term* caller)
    {
        Type* head = caller->input(0)->type;

        for (int nameIndex=1; nameIndex < caller->numInputs(); nameIndex++) {

            // Abort if input type is not correct
            if (!is_string(caller->input(1)))
                return &ANY_T;

            if (!is_list_based_type(head))
                return &ANY_T;

            std::string const& name = caller->input(1)->asString();

            int fieldIndex = list_find_field_index_by_name(head, name);

            if (fieldIndex == -1)
                return &ANY_T;

            head = as_type(list_get_type_list_from_type(head)->getIndex(fieldIndex));
        }

        return head;
    }

    void formatSource(StyledSource* source, Term* term)
    {
        format_name_binding(source, term);
        //append_phrase(source, get_relative_name(term, term->input(0)),
        //        term, phrase_type::UNDEFINED);
        format_source_for_input(source, term, 0);
        for (int i=1; i < term->numInputs(); i++) {
            append_phrase(source, ".", term, token::DOT);
            append_phrase(source, term->input(i)->asString(),
                    term, token::IDENTIFIER);
        }
    }

    void setup(Branch* kernel)
    {
        GET_FIELD_FUNC = import_function(kernel, evaluate,
                "get_field(any, string...) -> any");
        get_function_attrs(GET_FIELD_FUNC)->specializeType = specializeType;
        get_function_attrs(GET_FIELD_FUNC)->formatSource = formatSource;
    }
}
}
