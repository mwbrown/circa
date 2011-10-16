// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

namespace circa {
namespace length_function {

    CA_FUNCTION(length)
    {
        set_int(OUTPUT, num_elements(INPUT(0)));
    }
    void setup(Branch* kernel)
    {
        LENGTH_FUNC = import_function(kernel, length, "length(List) -> int");
    }
}
}
