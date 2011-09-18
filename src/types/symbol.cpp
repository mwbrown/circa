// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// Support for a symbol type, which is a string that is globally associated with
// an index.

namespace circa {
namespace symbol_t {

    int g_nextIndex = 1;
    std::map<std::string, int> g_nameToIndex;
    std::map<int, std::string> g_indexToName;

    std::string to_string(TaggedValue* value)
    {
        return ":" + g_indexToName[as_int(value)];
    }

    void assign(TaggedValue* result, const char* name)
    {
        std::map<std::string,int>::const_iterator it;
        it = symbol_t::g_nameToIndex.find(name);

        int index = 0;

        if (it == symbol_t::g_nameToIndex.end()) {
            index = g_nextIndex++;
            g_nameToIndex[name] = index;
            g_indexToName[index] = name;
        } else {
            index = it->second;
        }

        change_type_no_initialize(result, &SYMBOL_T);
        result->value_data.asint = index;
    }

    void setup_type(Type* type)
    {
        reset_type(type);
        type->name = "symbol";
        type->storageType = STORAGE_TYPE_INT;
        type->toString = to_string;
    }
}

} // namespace circa
