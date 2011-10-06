// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace preserve_state_result_function {

    CA_FUNCTION(preserve_state_result)
    {
#if 0
        TaggedValue result;
        copy(INPUT(0), &result);
        // TODO: switch to use consume_input instead of copy
        //consume_input(CALLER, 0, &result);

        // Use 'name' here, not uniqueName.
        const char* name = INPUT_TERM(0)->name.c_str();
        Dict* state = get_scope_state(CONTEXT, 0);
        swap(&result, state->insert(name));
        set_null(OUTPUT);
#endif
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, preserve_state_result, "preserve_state_result(any)");
        PRESERVE_STATE_RESULT_FUNC = kernel->get("preserve_state_result");
    }
}
}
