// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace live_refactoring_tests {

void test_freeze_state()
{
    Branch branch;
    Term* s = branch.compile("state s = 1");
    branch.compile("s += 1");

    EvalContext context;
    evaluate_branch(&context, &branch);
    evaluate_branch(&context, &branch);
    evaluate_branch(&context, &branch);

    test_equals(&context.state, "{s: 4}");
    test_equals(get_term_source_text(s), "state s = 1");

    branch.compile("freeze(s)");
    evaluate_branch(&context, &branch);

    test_equals(get_term_source_text(s), "s = 5");
}

void register_tests()
{
    REGISTER_TEST_CASE(live_refactoring_tests::test_freeze_state);
}

} // namespace live_refactoring_tests
} // namespace circa
