// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace cpp_importing {

void pointer_alloc(Term* type, Term* term)
{
    term->value_type = &as_type(type);
    term->value_data.ptr = NULL;
}

bool raw_value_less_than(Term* a, Term* b)
{
    return a->value_data.asint < b->value_data.asint;
}

} // namespace cpp_importing
} // namespace circa
