// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

#include "branch.h"
#include "builtins.h"
#include "bytecode.h"
#include "evaluation.h"
#include "testing.h"

namespace circa {
namespace runtime_tests {

void blocked_by_error()
{
    Branch branch;

    internal_debug_function::spy_clear();

    branch.compile("test_spy(1)");
    Term *error = branch.compile("e = assert(false)");
    branch.compile("test_spy(2)");

    EvalContext context;
    evaluate_save_locals(&context, &branch);
    test_assert(context.errorOccurred);
    test_assert(context.errorTerm == error);
    test_equals(internal_debug_function::spy_results(), "[1]");
}

void test_errored_function()
{
    Branch branch;
    branch.compile("e = assert(false)");
    Term* t = branch.compile("t = errored(e)");

    EvalContext context;
    evaluate_save_locals(&context, &branch);

    test_assert(!context.errorOccurred);
    test_assert(as_bool(t));
}

void test_misc()
{
    test_assert(is_type(TYPE_TYPE));
    test_equals(TYPE_TYPE->type->name, "Type");

    test_assert(is_type(FUNCTION_TYPE));
    test_equals(FUNCTION_TYPE->type->name, "Type");
}

void test_dont_crash_on_static_error()
{
    Branch branch;
    EvalContext context;
    branch.compile("nonexistant()");
    evaluate_branch_with_bytecode(&context, &branch);
    test_assert(has_static_errors(&branch));
}

void register_tests()
{
    REGISTER_TEST_CASE(runtime_tests::blocked_by_error);
    REGISTER_TEST_CASE(runtime_tests::test_errored_function);
    REGISTER_TEST_CASE(runtime_tests::test_misc);
    REGISTER_TEST_CASE(runtime_tests::test_dont_crash_on_static_error);
}

} // namespace runtime_tests

} // namespace circa
