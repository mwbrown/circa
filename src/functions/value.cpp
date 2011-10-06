// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace value_function {

    CA_FUNCTION(evaluate)
    {
        if (OUTPUT != NULL)
            copy(CALLER, OUTPUT);
    }

    void setup(Branch* kernel)
    {
    }
}
}
