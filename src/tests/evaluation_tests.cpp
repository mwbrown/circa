// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace evaluation_tests {

void test_branch_eval()
{
    Branch branch;
    branch.eval("a = 1");
}

void test_evaluate_minimum()
{
    Branch branch;
    Term* a = branch.compile("a = 1");
    Term* b = branch.compile("b = 2");
    Term* c = branch.compile("c = add(a b)");
    Term* d = branch.compile("d = sub(a b)");

    EvalContext context;
    TaggedValue result;
    evaluate_minimum_preserve_locals(&context, d, &result);

    test_equals(a, "1");
    test_equals(b, "2");
    test_equals(c, "null");
    test_equals(d, "-1");

    test_equals(&result, "-1");
}

void test_evaluate_minimum2()
{
    Branch branch;
    Term* a = branch.compile("a = [1]");
    Term* x = branch.compile("x = [2]");
    Term* b = branch.compile("b = [1 a 3]");
    Term* y = branch.compile("y = [a b]");
    Term* c = branch.compile("c = [b a]");
    Term* z = branch.compile("z = [a x b y]");
    Term* abc = branch.compile("evaluate_this = [a b c]");
    Term* xyz = branch.compile("dont_evaluate_this = [x y z]");

    EvalContext context;
    evaluate_minimum_preserve_locals(&context, abc, NULL);

    test_equals(a, "[1]");
    test_equals(b, "[1, [1], 3]");
    test_equals(c, "[[1, [1], 3], [1]]");
    test_equals(abc, "[[1], [1, [1], 3], [[1, [1], 3], [1]]]");
    test_equals(x, "null");
    test_equals(y, "null");
    test_equals(z, "null");
    test_equals(xyz, "null");
}

void test_evaluate_minimum_ignores_meta_inputs()
{
    Branch branch;
    Term* a = branch.compile("a = add(1 2)");
    Term* b = branch.compile("type(a)");
    EvalContext context;
    TaggedValue result;
    evaluate_minimum_preserve_locals(&context, b, &result);
    test_equals(a, "null");
    test_equals(as_type(&result)->name, "int");
}

void test_term_stack()
{
    Branch branch;
    branch.compile("def term_names(List names)->List { "
            "return for n in names { Ref(@n) n.name() } }");
    branch.compile("def f() { debug_get_term_stack() -> term_names -> test_spy }");
    branch.compile("def g() { f_call = f() }");
    branch.compile("def h() { g_call = g() }");

    internal_debug_function::spy_clear();
    branch.eval("h_call = h()");
    test_equals(internal_debug_function::spy_results()->get(0),
            "['h_call', 'g_call', 'f_call']");

    internal_debug_function::spy_clear();
    branch.eval("for_loop = for i in [0] { h_call = h() }");
    test_equals(internal_debug_function::spy_results()->get(0),
            "['for_loop', 'h_call', 'g_call', 'f_call']");

    internal_debug_function::spy_clear();
    branch.eval("if_block = if true { h_call = h() }");
    test_equals(internal_debug_function::spy_results()->get(0),
            "['if_block', '', 'h_call', 'g_call', 'f_call']");
}

void register_tests()
{
    REGISTER_TEST_CASE(evaluation_tests::test_branch_eval);
    REGISTER_TEST_CASE(evaluation_tests::test_evaluate_minimum);
    REGISTER_TEST_CASE(evaluation_tests::test_evaluate_minimum2);
    REGISTER_TEST_CASE(evaluation_tests::test_evaluate_minimum_ignores_meta_inputs);
    REGISTER_TEST_CASE(evaluation_tests::test_term_stack);
}

} // evaluation_tests
} // circa
