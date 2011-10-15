// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace subroutine_tests {

void test_return_simple()
{
    Branch branch;
    branch.compile("def f() -> number { return 4.0 }");
    Term* call = branch.compile("f()");

    test_assert(branch);
    interpret_save_locals(&branch);

    test_equals(call, "4.0");
}
void test_return_from_conditional()
{
    Branch branch;
    branch.compile("def my_max(number a, number b) -> number {"
                "  if (a < b) {"
                "    return(b)"
                "  } else { "
                "    return(a) }"
                "}");

    test_assert(branch);

    Term* call0 = branch.compile("my_max(3,8)");
    Term* call1 = branch.compile("my_max(3,3)");
    Term* call2 = branch.compile("my_max(11,8)");

    interpret_save_locals(&branch);

    test_equals(call0, "8.0");
    test_equals(call1, "3.0");
    test_equals(call2, "11.0");
}

void test_recursion()
{
    Branch branch;

    // I think this is the simplest possible recursive function. Evaluate it
    // just to make sure that nothing crashes.
    branch.compile("def recr(bool r) { if r { recr(false) }}");
    branch.eval("recr(true)");

    // Factorial
    branch.compile("def factorial(int n) -> int {"
                "  if (n < 2) {"
                "    return(1)"
                "  } else {"
                "    next_i = add_i(n, -1)"
                "    return(mult_i(n, factorial(next_i))) }}");

    TaggedValue* fact_1 = branch.eval("factorial(1)");
    test_assert(branch);
    test_assert(fact_1->asInt() == 1);

    TaggedValue* fact_2 = branch.eval("factorial(2)");
    test_assert(branch);
    test_assert(fact_2->asInt() == 2);

    TaggedValue* fact_3 = branch.eval("factorial(3)");
    test_assert(branch);
    test_assert(fact_3->asInt() == 6);

    TaggedValue* fact_4 = branch.eval("factorial(4)");
    test_assert(branch);
    test_assert(fact_4->asInt() == 24);

    branch.compile("def recr(int n) -> int\n"
                "  if (n == 1)\n"
                "    return(1)\n"
                "  else\n"
                "    return(recr(n - 1) + 1)\n");
    
    test_assert(branch.eval("recr(1)")->asInt() == 1);
    test_assert(branch.eval("recr(2)")->asInt() == 2);
    test_assert(branch.eval("recr(3)")->asInt() == 3);
    test_assert(branch.eval("recr(4)")->asInt() == 4);
}

void subroutine_stateful_term()
{
    EvalContext context;
    Branch branch;
    branch.compile("def mysub() { state a = 0.0 a += 1 }");

    // Make sure that stateful terms work correctly
    Term* call = branch.compile("call = mysub()");
    test_assert(call);
    test_assert(function_has_inlined_state(branch["mysub"]));

    interpret_save_locals(&context, &branch);

    test_equals(context.state.toString(), "{call: {a: 1.0}}");

    interpret_save_locals(&context, &branch);

    test_equals(context.state.toString(), "{call: {a: 2.0}}");

    // Make sure that subsequent calls to this subroutine have their own state container.
    branch.compile("another_call = mysub()");
    interpret_save_locals(&context, &branch);

    test_equals(context.state.toString(), "{another_call: {a: 1.0}, call: {a: 3.0}}");
}

void test_recursion_with_state()
{
    Branch branch;
    branch.compile("def recr(int i) -> int {"
                    "state s = test_oracle();"
                    "if i == 1 { return 1 }; return recr(i - 1) + 1"
                   "}");
    branch.compile("result = recr(4)");

    test_assert(branch);

    EvalContext context;

    internal_debug_function::oracle_clear();
    internal_debug_function::oracle_send(10);
    internal_debug_function::oracle_send(21);
    internal_debug_function::oracle_send(33);
    internal_debug_function::oracle_send(45);

    interpret_save_locals(&context, &branch);

    test_equals(&context.state,
        "{result: {_recr: {_recr: {_recr: {s: 45}, s: 33}, s: 21}, s: 10}}");
    test_equals(branch["result"], 4);
}

void shadow_input()
{
    Branch branch;

    // Try having a name that shadows an input. This had a bug at one point
    branch.eval("def f(int i) -> int { i = 2 return(i) }");

    TaggedValue* a = branch.eval("f(1)");

    test_assert(a->asInt() == 2);
}

void specialization_to_output_type()
{
    // If a subroutine is declared with an output type that is more specific
    // than the implicit output type, then make sure that it uses the
    // declared type. This code once had a bug.
    Branch branch;
    Term* a = branch.compile("def a() -> Point { return([1 2]) }");

    test_assert(function_get_output_type(a, 0)->name == "Point");

    // Make sure that the return value is preserved. This had a bug too
    TaggedValue* call = branch.eval("a()");
    test_assert(call->numElements() == 2);
    test_equals(call->getIndex(0)->toFloat(), 1.0);
    test_equals(call->getIndex(1)->toFloat(), 2.0);
}

void stateful_function_with_arguments()
{
    // This code once had a bug
    Branch branch;
    branch.compile("def myfunc(int i) -> int { state s; return(i) }");
    TaggedValue* call = branch.eval("myfunc(5)");
    test_assert(call->asInt() == 5);
}

void to_source_string()
{
    // This code once had a bug
    Branch branch;
    branch.compile("def f();");
    Term* c = branch.compile("f()");

    test_equals(get_term_source_text(c), "f()");
}

void bug_with_return()
{
    Branch branch;
    branch.compile("def f(bool b)->int { if b return 1 else return 2 }");
    Term* input = branch.compile("b = true");
    Term* f = branch.compile("f(b)");

    // there once was a bug where EvalContext.interruptSubroutine was not reset
    EvalContext context;
    interpret_save_locals(&context, &branch);
    test_assert(is_int(f));
    test_assert(f->asInt() == 1);

    set_bool(input, false);
    interpret_save_locals(&context, &branch);
    test_assert(is_int(f));
    test_assert(f->asInt() == 2);
}

void bug_where_interrupt_subroutine_wasnt_being_cleared()
{
    Branch branch;
    branch.compile("def f()->int { return(1) }");
    branch.compile("def g()->int { a = f() return(2) }");
    TaggedValue* x = branch.eval("x = g()");
    test_equals(x, 2);
}

namespace copy_counting_tests
{
    Type T;

    struct Slot
    {
        int copies;
        Slot() : copies(0) {}
    };

    const int num_slots = 10;
    int next_available_slot = 0;
    Slot slots[num_slots];

    void dump_slots(TaggedValue* value)
    {
        List* list = set_list(value, 0);
        for (int s=0; s < next_available_slot; s++) {
            set_int(list->append(), slots[s].copies);
        }
    }
    void dump_slots_stdout()
    {
        TaggedValue v;
        dump_slots(&v);
        std::cout << v.toString() << std::endl;
    }

    void t_initialize(Type* type, TaggedValue* source)
    {
        source->value_data.asint = next_available_slot++;
    }

    void t_copy(Type* type, TaggedValue* source, TaggedValue* dest)
    {
        change_type_no_initialize(dest, type);
        dest->value_data = source->value_data;
        Slot& slot = slots[dest->value_data.asint];
        slot.copies++;
    }

    void setup(Branch& branch)
    {
        T.name = "T";
        T.initialize = t_initialize;
        T.copy = t_copy;
        set_type(create_type(&branch, "T"), &T);
        for (int s = 0; s < num_slots; s++)
            slots[s] = Slot();
        next_available_slot = 0;
    }

    void test_single_call()
    {
        Branch branch;
        setup(branch);

        branch.compile("def f(T t);");
        Term* init = branch.compile("a = T()");
        Term* call = branch.compile("f(a)");
        test_assert(branch);
        test_assert(init->function != NULL);
        test_assert(call->function != NULL);

        int slot = next_available_slot - 1;

        test_equals(slots[slot].copies, 0);
        interpret_save_locals(&branch);
        // one copy for T(), another to evaluate f(a)
        test_assert(slots[slot].copies <= 2);
    }
}

void return_from_if_block()
{
    Branch branch;

    internal_debug_function::spy_clear();
    branch.compile("def f() { if true { test_spy(1); return; test_spy(2); }}");
    branch.eval("f()");
    test_equals(internal_debug_function::spy_results()->toString(), "[1]");

    internal_debug_function::spy_clear();
    branch.compile("def f() {if false {} else { test_spy(3); return; test_spy(4); }}");
    branch.eval("f()");
    test_equals(internal_debug_function::spy_results()->toString(), "[3]");
}

void return_from_for_loop()
{
    Branch branch;

#if 0
    internal_debug_function::spy_clear();
    branch.compile("def f() { for i in [1 2 3] { test_spy(5) return; test_spy(6) }}");
    branch.eval("f()");
    test_equals(internal_debug_function::spy_results()->toString(), "[5]");
#endif

    internal_debug_function::spy_clear();
    branch.compile("def f() { for i in [1 2 3 4] { "
            "test_spy(i) if i==3 { return }; test_spy(0) }}");

    branch.eval("f()");
    test_assert(branch);
    test_equals(internal_debug_function::spy_results()->toString(), "[1, 0, 2, 0, 3]");
}

void bug_with_misplaced_preserve_state_result()
{
    Branch branch;

    branch.compile("def f() {"
                    "if true { state s; s = 2 }"
                    "if true { state t; t = 3 }"
                   "}");

    // there was a bug where this would create extra preserve_state_result() calls.
    // These showed up as terms with null inputs.
    test_assert(branch);
}

void multiple_outputs()
{
    Branch branch;
    branch.compile("def inc(int i :out) { i += 1 }");
    branch.compile("x = 1");
    branch.compile("inc(&x)");
    interpret_save_locals(&branch);
    test_equals(branch["x"], 2);

    branch.clear();
    branch.compile("def inc3(int i :out, int j :out, int k :out) -> string "
            "{ i += 1; j += 2; k += 3; return 'hi' }");

    branch.compile("x = 1");
    branch.compile("y = 1");
    branch.compile("z = 1");
    branch.compile("result = inc3(&x, &y, &z)");
    interpret_save_locals(&branch);
    test_equals(branch["x"], 2);
    test_equals(branch["y"], 3);
    test_equals(branch["z"], 4);
    test_equals(branch["result"], "hi");
}

void register_tests()
{
    REGISTER_TEST_CASE(subroutine_tests::test_return_simple);
    REGISTER_TEST_CASE(subroutine_tests::test_return_from_conditional);
    REGISTER_TEST_CASE(subroutine_tests::test_recursion);
    REGISTER_TEST_CASE(subroutine_tests::subroutine_stateful_term);
    REGISTER_TEST_CASE(subroutine_tests::test_recursion_with_state);
    REGISTER_TEST_CASE(subroutine_tests::shadow_input);
    REGISTER_TEST_CASE(subroutine_tests::specialization_to_output_type);
    REGISTER_TEST_CASE(subroutine_tests::stateful_function_with_arguments);
    REGISTER_TEST_CASE(subroutine_tests::to_source_string);
    REGISTER_TEST_CASE(subroutine_tests::bug_with_return);
    REGISTER_TEST_CASE(subroutine_tests::bug_where_interrupt_subroutine_wasnt_being_cleared);
    REGISTER_TEST_CASE(subroutine_tests::copy_counting_tests::test_single_call);
    REGISTER_TEST_CASE(subroutine_tests::return_from_if_block);
    REGISTER_TEST_CASE(subroutine_tests::return_from_for_loop);
    REGISTER_TEST_CASE(subroutine_tests::bug_with_misplaced_preserve_state_result);
    REGISTER_TEST_CASE(subroutine_tests::multiple_outputs);
}

} // namespace refactoring_tests
} // namespace circa
