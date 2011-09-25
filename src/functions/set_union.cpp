// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../importing.h"
#include "../importing_macros.h"
#include "types/set.h"

namespace circa {
namespace set_union_function {

    CA_FUNCTION(evaluate)
    {
        List* list = List::checkCast(OUTPUT);
        list->clear();

        for (int inputIndex=0; inputIndex < NUM_INPUTS; inputIndex++) {
            List* input = List::checkCast(INPUT(inputIndex));
            int numElements = input->numElements();
            for (int i=0; i < numElements; i++)
                set_t::add(list, input->get(i));
        }
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate, "set_union(Set...) -> Set");
    }
}
}
