// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace switch_tests {

void test_simple()
{
    Branch branch;
    branch.compile("switch 1 { case 1 { test_spy(1) } }");

    testing_clear_spy();
    evaluate_save_locals(branch);
    test_equals(testing_get_spy_results(), "[1]");

    branch.clear();
    branch.compile("switch 2 { case 1 { test_spy(1) } }");
    testing_clear_spy();
    evaluate_save_locals(branch);
    test_equals(testing_get_spy_results(), "[]");

    branch.clear();
    branch.compile("switch 3 { case 2 { test_spy(1) } case 3 { test_spy(3) } }");
    testing_clear_spy();
    evaluate_save_locals(branch);
    test_equals(testing_get_spy_results(), "[3]");
}

void test_case_expressions_arent_constant()
{
    Branch branch;
    branch.compile("switch 2 { case 1+1 { test_spy(1) } }");

    testing_clear_spy();
    evaluate_save_locals(branch);
    test_equals(testing_get_spy_results(), "[1]");

    branch.clear();
    branch.compile("switch 2 { case 2+3 { test_spy(1) } }");
    testing_clear_spy();
    evaluate_save_locals(branch);
    test_equals(testing_get_spy_results(), "[]");

    branch.clear();
    branch.compile("switch 3 { case 5-1 { test_spy(1) } case 5-2 { test_spy(3) } }");
    testing_clear_spy();
    evaluate_save_locals(branch);
    test_equals(testing_get_spy_results(), "[3]");
}

void test_join_locals()
{
    Branch branch;
    branch.compile("a = 1");
    branch.compile("switch 2 { case 1 { a = 'wrong' } case 2 { a = 'right' } case 3 { a = 'alsoWrong' }}");

    testing_clear_spy();
    evaluate_save_locals(branch);
    test_equals(branch["a"], "right");
}

void register_tests()
{
    REGISTER_TEST_CASE(switch_tests::test_simple);
    REGISTER_TEST_CASE(switch_tests::test_case_expressions_arent_constant);
    REGISTER_TEST_CASE(switch_tests::test_join_locals);
}

}
} // namespace circa
