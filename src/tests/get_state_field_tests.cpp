// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace get_state_field_tests {

void initial_value_expr_1()
{
    Branch branch;
    branch.compile("state s = 1");

    EvalContext context;
    evaluate_save_locals(&context, &branch);
    test_equals(&context.state, "{s: 1}");
}

void initial_value_expr_2()
{
    Branch branch;
    branch.compile("local = add(1,1)");
    branch.compile("state s = local");
    
    EvalContext context;
    evaluate_save_locals(&context, &branch);
    test_equals(&context.state, "{s: 2}");
}

void initial_value_expr_3()
{
    Branch branch;
    branch.compile("state s = 1 + 1");

    EvalContext context;
    evaluate_save_locals(&context, &branch);
    test_equals(&context.state, "{s: 2}");
}

void register_tests()
{
    REGISTER_TEST_CASE(get_state_field_tests::initial_value_expr_1);
    REGISTER_TEST_CASE(get_state_field_tests::initial_value_expr_2);
    REGISTER_TEST_CASE(get_state_field_tests::initial_value_expr_3);
}

} // namespace get_state_field_tests
} // namespace circa
