// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../kernel.h"
#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace sub_function {

    CA_FUNCTION(evaluate_i)
    {
        set_int(OUTPUT, INT_INPUT(0) - INT_INPUT(1));
    }

    CA_FUNCTION(evaluate_f)
    {
        set_float(OUTPUT, FLOAT_INPUT(0) - FLOAT_INPUT(1));
    }

    void setup(Branch* kernel)
    {
        Term* sub_i = import_function(kernel, evaluate_i, "sub_i(int,int) -> int");
        Term* sub_f = import_function(kernel, evaluate_f, "sub_f(number,number) -> number");

        SUB_FUNC = create_overloaded_function(kernel, "sub", TermList(sub_i, sub_f));
    }
}
} // namespace circa
