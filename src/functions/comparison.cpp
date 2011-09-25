// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace comparison_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(less_than_i, "less_than_i(int,int) -> bool")
    {
        set_bool(OUTPUT, INT_INPUT(0) < INT_INPUT(1));
    }

    CA_DEFINE_FUNCTION(less_than_f, "less_than_f(number,number) -> bool")
    {
        set_bool(OUTPUT, FLOAT_INPUT(0) < FLOAT_INPUT(1));
    }

    CA_DEFINE_FUNCTION(less_than_eq_i, "less_than_eq_i(int,int) -> bool")
    {
        set_bool(OUTPUT, INT_INPUT(0) <= INT_INPUT(1));
    }

    CA_DEFINE_FUNCTION(less_than_eq_f, "less_than_eq_f(number,number) -> bool")
    {
        set_bool(OUTPUT, FLOAT_INPUT(0) <= FLOAT_INPUT(1));
    }

    CA_DEFINE_FUNCTION(greater_than_i, "greater_than_i(int,int) -> bool")
    {
        set_bool(OUTPUT, INT_INPUT(0) > INT_INPUT(1));
    }

    CA_DEFINE_FUNCTION(greater_than_f, "greater_than_f(number,number) -> bool")
    {
        set_bool(OUTPUT, FLOAT_INPUT(0) > FLOAT_INPUT(1));
    }

    CA_DEFINE_FUNCTION(greater_than_eq_i, "greater_than_eq_i(int,int) -> bool")
    {
        set_bool(OUTPUT, INT_INPUT(0) >= INT_INPUT(1));
    }

    CA_DEFINE_FUNCTION(greater_than_eq_f, "greater_than_eq_f(number,number) -> bool")
    {
        set_bool(OUTPUT, FLOAT_INPUT(0) >= FLOAT_INPUT(1));
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);

        create_overloaded_function(kernel, "less_than",
                TermList(kernel->get("less_than_i"), kernel->get("less_than_f")));
        create_overloaded_function(kernel, "less_than_eq",
                TermList(kernel->get("less_than_eq_i"), kernel->get("less_than_eq_f")));
        create_overloaded_function(kernel, "greater_than",
                TermList(kernel->get("greater_than_i"), kernel->get("greater_than_f")));
        create_overloaded_function(kernel, "greater_than_eq",
                TermList(kernel->get("greater_than_eq_i"), kernel->get("greater_than_eq_f")));
    }
}
}
