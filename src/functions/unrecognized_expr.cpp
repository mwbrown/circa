// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "circa.h"
#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace unrecognized_expr_function {

    CA_FUNCTION(evaluate)
    {
    }

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, term->stringProp("originalText"), term, phrase_type::UNDEFINED);
    }

    void setup(Branch* kernel)
    {
        UNRECOGNIZED_EXPRESSION_FUNC = import_function(kernel, evaluate, "unrecognized_expr(any...)");
        get_function_attrs(UNRECOGNIZED_EXPRESSION_FUNC)->formatSource = formatSource;
    }
}
}
