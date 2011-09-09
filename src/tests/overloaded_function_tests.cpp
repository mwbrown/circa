// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace overloaded_function_tests {

void simple_eval()
{
    Branch branch;
    branch.compile("a = add(3, 4.5)");
    evaluate_save_locals(branch);
    test_equals(branch["a"], "7.5");
}

void declared_in_script()
{
    Branch branch;
    branch.compile("def f_1(int i) -> int { return i + 1 }");
    branch.compile("def f_2(string s) -> string { return concat(s 'x') }");
    branch.compile("f = overloaded_function(f_1 f_2)");
    test_assert(branch);

    EvalContext context;
    evaluate_save_locals(&context, branch);
    test_assert(context);

    TaggedValue* a = branch.eval("a = f(1)");
    test_equals(a, 2);
    TaggedValue* b = branch.eval("b = f('aaa')");
    test_equals(b, "aaax");
}

void update_output_type()
{
    Branch branch;
    Term* f1 = branch.compile("def f1() -> int return 0");
    Term* f = overloaded_function::create_overloaded_function(branch, "f", TermList(f1));
    test_equals(function_get_output_type(f, 0)->name, "int");
    Term* f2 = branch.compile("def f2() -> string return ''");
    overloaded_function::append_overload(f, f2);
    test_equals(function_get_output_type(f, 0)->name, "any");
}

void test_specialize_type()
{
    Branch branch;
    Term* t = branch.compile("add(1,2)");
    test_equals(t->type->name, "int");
    Term* t2 = branch.compile("add(1.0,2.0)");
    test_equals(t2->type->name, "number");
}

void test_dynamic_overload()
{
    Branch branch;
    branch.compile("def f(bool b) -> any { if b { return 1 } else { return 1.0 } }");
    Term* a = branch.compile("a = add(f(true), f(true))");
    Term* b = branch.compile("b = add(f(false), f(false))");
    evaluate_save_locals(branch);
    test_equals(a, "2");
    test_equals(b, "2.0");
}

void register_tests()
{
    REGISTER_TEST_CASE(overloaded_function_tests::simple_eval);
    REGISTER_TEST_CASE(overloaded_function_tests::declared_in_script);
    REGISTER_TEST_CASE(overloaded_function_tests::update_output_type);
    REGISTER_TEST_CASE(overloaded_function_tests::test_specialize_type);
    REGISTER_TEST_CASE(overloaded_function_tests::test_dynamic_overload);
}

} // namespace overloaded_function_tests
} // namespace circa
