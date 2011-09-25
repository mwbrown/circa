// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace type_check_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(is_list, "is_list(any) -> bool")
    {
        set_bool(OUTPUT, circa::is_list(INPUT(0)));
    }
    CA_DEFINE_FUNCTION(is_int, "is_int(any) -> bool")
    {
        set_bool(OUTPUT, is_int(INPUT(0)));
    }
    CA_DEFINE_FUNCTION(is_float, "is_float(any) -> bool")
    {
        set_bool(OUTPUT, is_float(INPUT(0)));
    }
    CA_DEFINE_FUNCTION(is_bool, "is_bool(any) -> bool")
    {
        set_bool(OUTPUT, is_bool(INPUT(0)));
    }
    CA_DEFINE_FUNCTION(is_string, "is_string(any) -> bool")
    {
        set_bool(OUTPUT, is_string(INPUT(0)));
    }
    CA_DEFINE_FUNCTION(is_null, "is_null(any) -> bool")
    {
        set_bool(OUTPUT, is_null(INPUT(0)));
    }
    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }

} // namespace type_check_function
} // namespace circa
