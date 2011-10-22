// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "gc.h"
#include "object.h"

namespace circa {

void register_new_object(CircaObject* obj, Type* type)
{
    CircaObject* header = (CircaObject*) obj;

    obj->type = type;
    obj->next = NULL;
    obj->permanent = false;
    obj->gcColor = 0;
    gc_register_object(header);
}

void set_object_permanent(CircaObject* obj, bool permanent)
{
    obj->permanent = permanent;
}

} // namespace circa
