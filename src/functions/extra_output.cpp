// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace extra_output_function {
    void setup(Branch& kernel)
    {
        EXTRA_OUTPUT_FUNC = import_function(kernel, NULL, "extra_output(any) -> any");
    }
}
}
