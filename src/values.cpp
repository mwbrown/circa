// Copyright 2008 Paul Hodge

#include "common_headers.h"
#include "circa.h"
#include "values.h"

namespace circa {

void dealloc_value(Term* term)
{
    if (term->value == NULL)
        return;

    if (term->type->value == NULL)
        throw std::runtime_error("type is undefined");

    if (as_type(term->type)->dealloc == NULL)
        throw std::runtime_error("type " + as_type(term->type)->name
            + " has no dealloc function");

    as_type(term->type)->dealloc(term);
    term->value = NULL;
}

void recycle_value(Term* source, Term* dest)
{
    if (source->type != dest->type)
        throw errors::TypeError(dest, source->type);

    // Only steal if the term says it's OK
    bool steal = source->stealingOk;

    // Don't steal from types
    if (source->type == TYPE_TYPE)
        steal = false;

    if (steal)
        steal_value(source, dest);
    else
        duplicate_value(source, dest);
}

void duplicate_value(Term* source, Term* dest)
{
    if (source == dest)
        throw std::runtime_error("in duplicate_value, can't have source == dest");

    if (source->type != dest->type)
        throw errors::TypeError(dest, source->type);

    Type::DuplicateFunc duplicate = as_type(source->type)->duplicate;

    if (duplicate == NULL)
        throw std::runtime_error(std::string("type ") + as_type(source->type)->name
                + " has no duplicate function");

    dealloc_value(dest);

    duplicate(source, dest);
}

void steal_value(Term* source, Term* dest)
{
    if (source->type != dest->type)
        throw errors::TypeError(dest, source->type);

    // if 'dest' has a value, delete it
    dealloc_value(dest);

    dest->value = source->value;

    source->value = NULL;
    source->needsUpdate = true;
}

} // namespace circa
