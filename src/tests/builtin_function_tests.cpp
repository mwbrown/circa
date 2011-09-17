// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include <circa.h>
#include "importing_macros.h"

namespace circa {
namespace builtin_function_tests {

void test_int()
{
    Branch branch;

    test_assert(INT_T.formatSource != NULL);

    Term* four = create_int(branch, 4);
    Term* another_four = create_int(branch, 4);
    Term* five = create_int(branch, 5);

    test_assert(equals(four, another_four));
    test_assert(!equals(four, five));

    test_equals(four->toString(), "4");
}

void test_float()
{
    Branch branch;

    Type* floatType = &FLOAT_T;

    test_assert(floatType->equals != NULL);
    test_assert(floatType->formatSource != NULL);

    Term* point_one = create_float(branch, .1f);
    Term* point_one_again = create_float(branch, .1f);
    Term* point_two = create_float(branch, 0.2f);

    test_assert(equals(point_one, point_one_again));
    test_assert(equals(point_two, point_two));

    test_equals(point_one->toString(), "0.1");
}

void test_bool()
{
    Branch branch;

    test_assert(as_string(branch.eval("cond(true, 'a', 'b')")) == "a");
    test_assert(as_string(branch.eval("cond(false, 'a', 'b')")) == "b");
}

void test_builtin_equals()
{
    Branch branch;
    EvalContext context;
    branch.compile("equals(5.0, 'hello')");
    evaluate_save_locals(&context, branch);
}

void test_list()
{
    Branch branch;
    TaggedValue* l = branch.eval("l = list(1,2,'foo')");

    test_assert(l->getIndex(0)->asInt() == 1);
    test_assert(l->getIndex(1)->asInt() == 2);
    test_assert(l->getIndex(2)->asString() == "foo");
}

void test_vectorized_funcs()
{
    Branch branch;
    TaggedValue* t = branch.eval("[1 2 3] + [4 5 6]");
    test_assert(t);

    test_assert(t->numElements() == 3);
    test_assert(t->getIndex(0)->asInt() == 5);
    test_assert(t->getIndex(1)->asInt() == 7);
    test_assert(t->getIndex(2)->asInt() == 9);

    // Test mult_s (multiply a vector to a scalar)
    t = branch.eval("[10 20 30] * 1.1");

    test_assert(t->numElements() == 3);
    test_equals(t->getIndex(0)->asFloat(), 11);
    test_equals(t->getIndex(1)->asFloat(), 22);
    test_equals(t->getIndex(2)->asFloat(), 33);

    // Test error handling
    EvalContext context;
    evaluate(&context, branch, "[1 1 1] + [1 1]");
    test_assert(context.errorOccurred);
}

void test_vectorized_funcs_with_points()
{
    // This test is similar, but we define the type Point and see if
    // vectorized functions work against that.
    Branch branch;
    
    Term* point_t = branch.compile("type Point {number x, number y}");

    Term* a = branch.compile("a = [1 0] -> Point");

    test_assert(a->type == as_type(point_t));

    Term* b = branch.compile("b = a + [0 2]");

    evaluate_save_locals(branch);

    test_equals(b->getIndex(0)->toFloat(), 1);
    test_equals(b->getIndex(1)->toFloat(), 2);
}

void test_cond_with_int_and_float()
{
    Branch branch;

    // This code once caused a bug
    Term* a = branch.compile("cond(true, 1, 1.0)");
    test_assert(a->type != &ANY_T);
    Term* b = branch.compile("cond(true, 1.0, 1)");
    test_assert(b->type != &ANY_T);
}

void test_get_index()
{
    Branch branch;
    branch.eval("l = [1 2 3]");
    TaggedValue* get = branch.compile("get_index(l, 0)");

    evaluate_save_locals(branch);

    test_assert(get->value_type == unbox_type(INT_TYPE));
    test_assert(get->asInt() == 1);

    EvalContext context;
    evaluate(&context, branch, "l = []; get_index(l, 5)");
    test_assert(context.errorOccurred);
}

void test_set_index()
{
    Branch branch;

    branch.eval("l = [1 2 3]");
    TaggedValue* l2 = branch.compile("set_index(@l, 1, 5)");

    evaluate_save_locals(branch);
    test_assert(l2->getIndex(0)->asInt() == 1);
    test_assert(l2->getIndex(1)->asInt() == 5);
    test_assert(l2->getIndex(2)->asInt() == 3);

    TaggedValue* l3 = branch.compile("l[2] = 9");
    evaluate_save_locals(branch);
    test_assert(l3->getIndex(0)->asInt() == 1);
    test_assert(l3->getIndex(1)->asInt() == 5);
    test_assert(l3->getIndex(2)->asInt() == 9);
}

void test_do_once()
{
    Branch branch;
    EvalContext context;

    Term* x = branch.compile("x = 1");
    Term* t = branch.compile("do_once()");
    nested_contents(t).compile("unsafe_assign(x,2)");

    test_assert(branch);

    test_assert(as_int(x) == 1);

    // the assign() inside do_once should modify x
    evaluate_save_locals(&context, branch);
    test_assert(as_int(x) == 2);

    // but if we call it again, it shouldn't do that any more
    set_int(x, 3);
    evaluate_save_locals(&context, branch);
    test_assert(as_int(x) == 3);
}

void test_changed()
{
    Branch branch;
    EvalContext context;
    Term* x = branch.compile("x = 5");
    Term* changed = branch.compile("changed(x)");

    evaluate_save_locals(&context, branch);
    test_assert(changed->asBool() == true);

    evaluate_save_locals(&context, branch);
    test_assert(changed->asBool() == false);
    evaluate_save_locals(&context, branch);
    test_assert(changed->asBool() == false);

    set_int(x, 6);
    evaluate_save_locals(&context, branch);
    test_assert(changed->asBool() == true);
    evaluate_save_locals(&context, branch);
    test_assert(changed->asBool() == false);
}

void test_delta()
{
    Branch branch;

    Term* i = branch.compile("i = 0");
    Term* delta = branch.compile("delta(i)");

    EvalContext context;
    evaluate_save_locals(&context, branch);

    test_assert(is_float(delta));
    test_equals(delta->toFloat(), 0);
    
    set_int(i, 5);
    evaluate_save_locals(&context, branch);
    test_assert(is_float(delta));
    test_equals(delta->toFloat(), 5);

    // do another evaluation without changing i, delta is now 0
    evaluate_save_locals(&context, branch);
    test_equals(delta->toFloat(), 0);

    set_int(i, 2);
    evaluate_save_locals(&context, branch);
    test_equals(delta->toFloat(), -3);

    set_int(i, 0);
    evaluate_save_locals(&context, branch);
    test_equals(delta->toFloat(), -2);
}

void test_message_passing()
{
    Branch branch;
    EvalContext context;
    Term* inbox = branch.compile("inbox = receive('inbox_name')");
    Term* send = branch.compile("send('inbox_name', 1)");

    // Before running, message queue should be empty
    test_assert(inbox->numElements() == 0);
    test_equals(&context.messages, "{}");

    // First run, inbox is still empty, but there is 1 message in transit
    evaluate_save_locals(&context, branch);
    test_assert(inbox->numElements() == 0);
    test_equals(&context.messages, "{inbox_name: [1]}");

    // Second run, inbox now returns 1
    evaluate_save_locals(&context, branch);
    test_assert(inbox->numElements() == 1);
    test_assert(inbox->getIndex(0)->asInt() == 1);
    test_equals(&context.messages, "{inbox_name: [1]}");

    // Delete the send() call
    remove_term(send);

    // Third run, inbox still returns 1 (from previous call), message queue is empty
    evaluate_save_locals(&context, branch);
    test_assert(inbox->numElements() == 1);
    test_assert(inbox->getIndex(0)->asInt() == 1);
    test_equals(&context.messages, "{inbox_name: []}");

    // Fourth run, inbox is empty again
    evaluate_save_locals(&context, branch);
    test_assert(inbox->numElements() == 0);
    test_equals(&context.messages, "{inbox_name: []}");
}

void test_message_passing2()
{
    // Repro a bug from plastic's runtime
    Branch branch;
    branch.compile(
        "state last_output = 1\n"
        "incoming = receive('inbox_name')\n"
        "def send_func(any s)\n"
        "  send('inbox_name', s)\n"
        "for s in incoming\n"
        "  last_output = s\n"
        "send_func(2)\n");

    EvalContext context;
    evaluate_save_locals(&context, branch);

    test_equals(&context.state, "{last_output: 1}");

    evaluate_save_locals(&context, branch);
    test_equals(&context.state, "{last_output: 2}");

    evaluate_save_locals(&context, branch);
    test_equals(&context.state, "{last_output: 2}");

    evaluate_save_locals(&context, branch);
    test_equals(&context.state, "{last_output: 2}");
}

void test_run_single_statement()
{
    Branch branch;
    branch.compile("br = { test_spy(1) test_spy('two') test_spy(3) \n"
        "-- this is a comment \n"
        "\n"
        "test_spy(4) }");

    internal_debug_function::spy_clear();
    branch.compile("run_single_statement(br, 0)");
    evaluate_save_locals(branch);
    test_equals(internal_debug_function::spy_results(), "[1]");

    internal_debug_function::spy_clear();
    branch.eval("run_single_statement(br, 1)");
    test_equals(internal_debug_function::spy_results(), "['two']");

    internal_debug_function::spy_clear();
    branch.eval("run_single_statement(br, 2)");
    test_equals(internal_debug_function::spy_results(), "[3]");

    internal_debug_function::spy_clear();
    branch.eval("run_single_statement(br, 3)");
    test_equals(internal_debug_function::spy_results(), "[4]");
}

void test_type_func()
{
    Branch branch;
    test_assert(as_type(branch.eval("type(1)")) == &INT_T);
    test_assert(as_type(branch.eval("type(1.0)")) == &FLOAT_T);
    test_assert(as_type(branch.eval("type('hi')")) == &STRING_T);
}

void test_typename_func()
{
    Branch branch;
    test_equals(branch.eval("typename(1)"), "int");
    test_equals(branch.eval("typename(int)"), "Type");
    test_equals(branch.eval("typename('hi')"), "string");
}

void register_tests()
{
    REGISTER_TEST_CASE(builtin_function_tests::test_int);
    REGISTER_TEST_CASE(builtin_function_tests::test_float);
    REGISTER_TEST_CASE(builtin_function_tests::test_bool);
    REGISTER_TEST_CASE(builtin_function_tests::test_builtin_equals);
    REGISTER_TEST_CASE(builtin_function_tests::test_list);
    REGISTER_TEST_CASE(builtin_function_tests::test_vectorized_funcs);
    REGISTER_TEST_CASE(builtin_function_tests::test_vectorized_funcs_with_points);
    REGISTER_TEST_CASE(builtin_function_tests::test_cond_with_int_and_float);
    REGISTER_TEST_CASE(builtin_function_tests::test_get_index);
    REGISTER_TEST_CASE(builtin_function_tests::test_set_index);
    //TEST_DISABLED REGISTER_TEST_CASE(builtin_function_tests::test_do_once);
    REGISTER_TEST_CASE(builtin_function_tests::test_changed);
    REGISTER_TEST_CASE(builtin_function_tests::test_delta);
    REGISTER_TEST_CASE(builtin_function_tests::test_message_passing);
    REGISTER_TEST_CASE(builtin_function_tests::test_message_passing2);
    REGISTER_TEST_CASE(builtin_function_tests::test_run_single_statement);
    REGISTER_TEST_CASE(builtin_function_tests::test_type_func);
    REGISTER_TEST_CASE(builtin_function_tests::test_typename_func);
}

} // namespace builtin_function_tests

} // namespace circa
