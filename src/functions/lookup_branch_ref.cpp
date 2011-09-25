// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace lookup_branch_ref_function {

    CA_FUNCTION(evaluate)
    {
        #if 0
        std::string name = as_string(INPUT(0));
        Term* term = get_global(name);

        if (term == NULL)
            return branch_ref_t::set_from_ref(OUTPUT, NULL);

        // FIXME: Don't give references to a non-branch

        return branch_ref_t::set_from_ref(OUTPUT, term);
        #endif
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate, "lookup_branch_ref(string) -> Branch");
    }
}
}
