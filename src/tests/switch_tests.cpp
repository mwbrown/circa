// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace switch_tests {

void test_simple()
{
    Branch branch;
    branch.compile("switch 1 { case 1 { test_spy(1) } }");
    internal_debug_function::spy_clear();
    evaluate_save_locals(branch);
    test_equals(internal_debug_function::spy_results(), "[1]");

    branch.clear();
    branch.compile("switch 2 { case 1 { test_spy(1) } }");
    internal_debug_function::spy_clear();
    evaluate_save_locals(branch);
    test_equals(internal_debug_function::spy_results(), "[]");

    branch.clear();
    branch.compile("switch 3 { case 2 { test_spy(1) } case 3 { test_spy(3) } }");
    internal_debug_function::spy_clear();
    evaluate_save_locals(branch);
    test_equals(internal_debug_function::spy_results(), "[3]");

}

void register_tests()
{
    REGISTER_TEST_CASE(switch_tests::test_simple);
}

}
} // namespace circa
