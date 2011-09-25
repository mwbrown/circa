// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include <cassert>

#include "build_options.h"
#include "branch.h"
#include "bytecode.h"
#include "introspection.h"

#include "debug.h"

namespace circa {

bool DEBUG_TRAP_NAME_LOOKUP = false;
bool DEBUG_TRAP_ERROR_OCCURRED = false;
bool DEBUG_TRACE_ALL_REF_WRITES = false;
bool DEBUG_TRACE_ALL_TERM_DESTRUCTORS = false;

void dump(Branch* branch)
{
    print_branch(std::cout, branch);
}

void dump_with_props(Branch* branch)
{
    print_branch_with_properties(std::cout, branch);
}

void dump(TaggedValue& value)
{
    std::cout << value.toString() << std::endl;
}
void dump(TaggedValue* value)
{
    std::cout << value->toString() << std::endl;
}

void dump(BytecodeData* bytecode)
{
    if (bytecode == NULL)
        std::cout << "<NULL bytecode>" << std::endl;
    print_bytecode(bytecode, std::cout);
}

void internal_error(const char* message)
{
    #if CIRCA_ASSERT_ON_ERROR
        std::cerr << "internal error: " << message << std::endl;
        assert(false);
    #else
        throw std::runtime_error(message);
    #endif
}

void internal_error(std::string const& message)
{
    internal_error(message.c_str());
}

void ca_assert_function(bool expr, const char* exprStr, int line, const char* file)
{
    if (!expr) {
        std::stringstream msg;
        msg << "ca_assert(" << exprStr << ") failed in " << file << " line " << line;
        internal_error(msg.str().c_str());
    }
}

} // namespace circa
