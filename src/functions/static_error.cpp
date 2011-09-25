// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../kernel.h"
#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace static_error_function {

    void setup(Branch* kernel)
    {
        STATIC_ERROR_FUNC = 
            import_function(kernel, NULL, "static_error(any msg)");
    }

}
}
