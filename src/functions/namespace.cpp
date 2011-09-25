// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "circa.h"
#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace namespace_function {

    CA_FUNCTION(evaluate)
    {
        evaluate_branch_internal_with_state(CONTEXT, CALLER, nested_contents(CALLER));
    }

    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, "namespace ", term, phrase_type::KEYWORD);
        append_phrase(source, term->name, term, phrase_type::TERM_NAME);
        format_branch_source(source, nested_contents(term), term);
        append_phrase(source, term->stringPropOptional("syntax:preEndWs", ""),
                term, token::WHITESPACE);
    }

    void early_setup(Branch* kernel)
    {
        NAMESPACE_FUNC = import_function(kernel, evaluate, "namespace()");
        get_function_attrs(NAMESPACE_FUNC)->formatSource = format_source;
    }
    void setup(Branch* kernel) {}
}
}
