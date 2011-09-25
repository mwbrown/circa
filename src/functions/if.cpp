// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../builtins.h"
#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace if_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(evaluate, "if(bool) -> any")
    {
        // Compilation placeholder
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        IF_FUNC = kernel->get("if");
        JOIN_FUNC = kernel->get("join");
    }
}
}
