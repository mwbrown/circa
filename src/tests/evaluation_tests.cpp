// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace evaluation_tests {

void test_manual_evaluate_branch()
{
#if 0 // TEST_DISABLED
    Branch branch;
    branch.compile("add_i(1,2)");

    EvalContext context;
    push_stack_frame(&context, &branch);
    evaluate_branch_with_bytecode(&context, &branch);

    // TODO: This will change when local indexes are condensed:
    test_equals(&context.stack, "[[null, null, 3]]");

    pop_stack_frame(&context);
    test_equals(&context.stack, "[]");

    branch.clear();
    branch.compile("a = add_i(1,2)");
    branch.compile("b = mult_i(a,2)");

    push_stack_frame(&context, &branch);
    evaluate_branch_with_bytecode(&context, &branch);

    test_equals(&context.stack, "[[null, null, 3, null, 6]]");
    pop_stack_frame(&context);
#endif
}

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
    Term* c = branch.compile("test_spy(a b)");
    Term* d = branch.compile("d = sub(a b)");

    internal_debug_function::spy_clear();
    test_equals(internal_debug_function::spy_results(), "[]");

    EvalContext context;
    context.preserveLocals = true;
    TaggedValue result;
    evaluate_minimum(&context, d, &result);

    test_equals(internal_debug_function::spy_results(), "[]");

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
    context.preserveLocals = true;
    evaluate_minimum(&context, abc, NULL);

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
    context.preserveLocals = true;
    evaluate_minimum(&context, b, &result);
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
            "['if_block', 'h_call', 'g_call', 'f_call']");
}

void evaluate_range_remapped_locals()
{
    // Call evaluate_range() using some terms that have non-evaluated terms as
    // input. The evaluation should use those term's global values instead.
    
    Branch branch;
    EvalContext context;

    Term* a = branch.compile("a = add_i(1, 2)");
    int a_pos = branch.length();

    evaluate_range(&context, &branch, 0, a_pos);
    test_equals(a, "3");

    Term* b = branch.compile("b = add_i(a, a)");
    evaluate_range(&context, &branch, a_pos, branch.length());
    test_equals(b, "6");

    // Do the same test but cheat; insert a fake result for 'a' and make
    // sure that it is used.
    set_int(a, 7);
    evaluate_range(&context, &branch, a_pos, branch.length());
    test_equals(b, "14");
}

void eval_context_inputs()
{
    EvalContext context;
    set_int(context.argumentList.append(), 3);

    Branch branch;
    branch.compile("test_spy(input(0))");
    testing_clear_spy();
    evaluate_branch(&context, &branch);
    test_equals(testing_get_spy_results(), "[3]");
}

void register_tests()
{
    REGISTER_TEST_CASE(evaluation_tests::test_manual_evaluate_branch);
    REGISTER_TEST_CASE(evaluation_tests::test_branch_eval);
    REGISTER_TEST_CASE(evaluation_tests::test_evaluate_minimum);
    REGISTER_TEST_CASE(evaluation_tests::test_evaluate_minimum2);
    REGISTER_TEST_CASE(evaluation_tests::test_evaluate_minimum_ignores_meta_inputs);
    REGISTER_TEST_CASE(evaluation_tests::test_term_stack);
    REGISTER_TEST_CASE(evaluation_tests::evaluate_range_remapped_locals);
    REGISTER_TEST_CASE(evaluation_tests::eval_context_inputs);
}

} // evaluation_tests
} // circa
