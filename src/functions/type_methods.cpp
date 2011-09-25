// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../importing.h"
#include "../importing_macros.h"
#include "../type.h"

namespace circa {
namespace type_methods_function {

    CA_FUNCTION(name_accessor)
    {
        set_string(OUTPUT, as_type(INPUT(0))->name);
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, name_accessor, "Type.name(self) -> string");
    }
}
}
