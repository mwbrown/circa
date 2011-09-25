// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../importing.h"
#include "../importing_macros.h"
#include "importing.h"

namespace circa {
namespace any_true_function {

    CA_FUNCTION(any_true)
    {
        TaggedValue* input = INPUT(0);

        int numElements = input->numElements();

        bool result = false;
        for (int i=0; i < numElements; i++)
            if (as_bool((*input)[i])) {
                result = true;
                break;
            }

        set_bool(OUTPUT, result);
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, any_true, "any_true(List l) -> bool;"
                "'Return whether any of the items in l are true'");
    }
}
} // namespace circa
