// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

struct CircaObject
{
    Type* type;

    CircaObject* next;

    // if 'permanent' is true then this object can't be garbage collected.
    bool permanent;

    // Used during GC collection
    char gcColor;
};

void register_new_object(CircaObject* obj, Type* type);
void set_object_permanent(CircaObject* obj, bool permanent);

} // namespace circa
