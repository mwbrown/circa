// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../importing.h"
#include "../refactoring.h"

namespace circa {
namespace codemod_functions {

    // Take the current local value of the given term, and transform that term
    // into a value.
    CA_FUNCTION(freeze)
    {
        TaggedValue* value = INPUT(0);

        Term* term = INPUT_TERM(0);

        // If this term is a name rebind, walk upwards until we get to the root name.
        while (term->numInputs() > 0
                && term->input(0) != NULL
                && term->input(0)->name == term->name)
            term = term->input(0);

        if (is_value(term))
            return;

        change_function(term, VALUE_FUNC);
        change_declared_type(term, value->value_type);
        copy(value, term);
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, freeze, "freeze(any)");
    }
}
}
