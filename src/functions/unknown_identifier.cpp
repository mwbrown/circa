// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "circa.h"
#include "../importing.h"
#include "../importing_macros.h"
#include "kernel.h"

namespace circa {
namespace unknown_identifier_function {

    CA_FUNCTION(evaluate)
    {
    }

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, term->name, term, phrase_type::UNKNOWN_IDENTIFIER);
    }

    void setup(Branch* kernel)
    {
        UNKNOWN_IDENTIFIER_FUNC = import_function(kernel, evaluate, "unknown_identifier() -> any");
        get_function_attrs(UNKNOWN_IDENTIFIER_FUNC)->formatSource = formatSource;
    }
}
}
