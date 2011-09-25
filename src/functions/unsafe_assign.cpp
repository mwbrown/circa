// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "circa.h"
#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace unsafe_assign_function {

    CA_FUNCTION(evaluate)
    {
        // The thing we are changing is on the left, the desired value is on the right
        // This is a little confusing because the C function 'cast' is the other
        // way around. The reason we have this order is because the infix operator :=
        // arranges its inputs as destination := source.
        Term* destination = INPUT_TERM(0);
        Term* source = INPUT_TERM(1);

        if (!cast_possible(source, declared_type(destination))) {
            error_occurred(CONTEXT, CALLER,
                    "Tried to assign a " + source->type->name + " to a "
                    + destination->type->name);
            return;
        }

        cast(source, declared_type(destination), destination);
    }

    void setup(Branch* kernel)
    {
        UNSAFE_ASSIGN_FUNC = import_function(kernel, evaluate, "unsafe_assign(any, any)");
    }
}
}
