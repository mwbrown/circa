// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "circa.h"
#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace instance_function {
    
    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(instance, "instance(Type t) -> any")
    {
        change_type(OUTPUT, unbox_type(INPUT(0)));
    }

    Type* specializeType(Term* caller)
    {
        return as_type(caller->input(0));
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        INSTANCE_FUNC = kernel->get("instance");
        get_function_attrs(INSTANCE_FUNC)->specializeType = specializeType;
    }
}
} // namespace circa
