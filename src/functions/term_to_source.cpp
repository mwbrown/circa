// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "circa.h"
#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace term_to_source_function {

    CA_FUNCTION(evaluate)
    {
        Term* term = INPUT_TERM(0);
        set_string(OUTPUT, get_term_source_text(term));
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate, "term_to_source(any :meta) -> string");
    }
}
} // namespace circa
