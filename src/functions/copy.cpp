// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "circa.h"

#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace copy_function {

    CA_FUNCTION(evaluate)
    {
        CONSUME_INPUT(0, OUTPUT);
    }

    Type* specializeType(Term* caller)
    {
        return get_type_of_input(caller, 0);
    }

    void formatSource(StyledSource* source, Term* term)
    {
        format_name_binding(source, term);
        append_phrase(source, get_relative_name(term, term->input(0)),
                term, token::IDENTIFIER);
    }

    void setup(Branch* kernel)
    {
        COPY_FUNC = import_function(kernel, evaluate, "copy(any) -> any");
        get_function_attrs(COPY_FUNC)->specializeType = specializeType;
        get_function_attrs(COPY_FUNC)->formatSource = formatSource;
    }
}
}
