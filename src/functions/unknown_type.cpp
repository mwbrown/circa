// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../builtins.h"
#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace unknown_type_function {

    CA_FUNCTION(evaluate)
    {
    }

    void setup(Branch* kernel)
    {
        UNKNOWN_TYPE_FUNC = import_function(kernel, evaluate, "unknown_type() -> Type");
    }
}
}
