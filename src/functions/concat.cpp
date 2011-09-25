// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace concat_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(concat, "concat(any...) -> string;"
            "'Concatenate each input (converting to a string if necessary).'")
    {
        std::stringstream out;
        for (int index=0; index < NUM_INPUTS; index++) {
            TaggedValue* v = INPUT(index);
            if (is_string(v))
                out << as_string(v);
            else
                out << to_string(v);
        }
        set_string(OUTPUT, out.str());
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
