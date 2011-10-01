// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "../interpreter.h"

namespace circa {
namespace interpreter_tests {

void test_simple()
{
    Branch branch;
    branch.compile("test_spy(1)");
    testing_clear_spy();

    EvalContext context;
    interpret(&context, &branch);

    test_equals(testing_get_spy_results(), "[1]");
}

void test_if_simple()
{
    Branch branch;
    branch.compile("if false { test_spy('f') }");
    branch.compile("if true { test_spy('t') }");

    EvalContext context;
    testing_clear_spy();
    interpret(&context, &branch);

    test_equals(testing_get_spy_results(), "['t']");
}

void test_for_loop_simple()
{
    Branch branch;
    branch.compile("for i in [1 2 3] { test_spy(i) }");

    EvalContext context;
    testing_clear_spy();
    interpret(&context, &branch);

    test_equals(testing_get_spy_results(), "[1, 2, 3]");
}

void test_for_loop_control_flow()
{
    Branch branch;
    EvalContext context;

    branch.compile("for i in [1 2] { test_spy(1); break; test_spy(2) }");
    testing_clear_spy();
    interpret(&context, &branch);
    test_equals(testing_get_spy_results(), "[1]");
    branch.clear();

    branch.compile("for i in [1 2] { test_spy(1); continue; test_spy(2) }");
    testing_clear_spy();
    interpret(&context, &branch);
    test_equals(testing_get_spy_results(), "[1, 1]");
    branch.clear();
}

void test_interpret_range_simple()
{
    Branch branch;

    Term* a = branch.compile("a = add_i(1 1)");
    Term* b = branch.compile("b = add_i(2 2)");
    Term* c = branch.compile("c = add_i(3 3)");

    EvalContext context;
    interpret_range(&context, &branch, b->index, c->index);
    test_equals(a, "null");
    test_equals(b, "4");
    test_equals(c, "null");
}

void register_tests()
{
    REGISTER_TEST_CASE(interpreter_tests::test_simple);
    REGISTER_TEST_CASE(interpreter_tests::test_if_simple);
    REGISTER_TEST_CASE(interpreter_tests::test_for_loop_simple);
    REGISTER_TEST_CASE(interpreter_tests::test_for_loop_control_flow);
    REGISTER_TEST_CASE(interpreter_tests::test_interpret_range_simple);
}

}
} // namespace circa
