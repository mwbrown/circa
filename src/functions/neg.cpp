// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "circa.h"
#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace neg_function  {

    CA_FUNCTION(evaluate_f)
    {
        set_float(OUTPUT, -FLOAT_INPUT(0));
    }

    CA_FUNCTION(evaluate_i)
    {
        set_int(OUTPUT, -INT_INPUT(0));
    }

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "-", term, phrase_type::INFIX_OPERATOR);
        format_source_for_input(source, term, 0);
    }

    void setup(Branch* kernel)
    {
        Term* neg_i = import_function(kernel, evaluate_i, "neg_i(int) -> int");
        Term* neg_f = import_function(kernel, evaluate_f, "neg_f(number) -> number");

        get_function_attrs(neg_i)->formatSource = formatSource;
        get_function_attrs(neg_f)->formatSource = formatSource;

        NEG_FUNC = create_overloaded_function(kernel, "neg", TermList(neg_i, neg_f));
        get_function_attrs(NEG_FUNC)->formatSource = formatSource;
    }
}
}
