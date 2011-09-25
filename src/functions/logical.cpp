// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace logical_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(and, "and(bool a, bool b) -> bool;"
                "'Return whether a and b are both true'")
    {
        set_bool(OUTPUT, as_bool(INPUT(0)) && as_bool(INPUT(1)));
    }

    CA_DEFINE_FUNCTION(or, "or(bool a, bool b) -> bool;"
                "'Return whether a or b are both true'")
    {
        set_bool(OUTPUT, as_bool(INPUT(0)) || as_bool(INPUT(1)));
    }

    CA_DEFINE_FUNCTION(not, "not(bool) -> bool")
    {
        set_bool(OUTPUT, !BOOL_INPUT(0));
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
