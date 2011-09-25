// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "circa.h"
#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace ref_function {

    CA_FUNCTION(ref)
    {
        set_ref(OUTPUT, INPUT_TERM(0));
    }

    void setup(Branch* kernel)
    {
        REF_FUNC = import_function(kernel, ref, "ref(any :ignore_error) -> Ref");
    }
}
} // namespace circa
