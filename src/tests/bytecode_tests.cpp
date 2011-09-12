// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "bytecode.h"
#include "testing.h"

namespace circa {
namespace bytecode_tests {

void test_simple_write()
{
    Branch branch;
    Term* a = branch.compile("add(1,2)");
    Term* b = branch.compile("mult(3,4)");
    BytecodeWriter writer;

    bc_write_call_op(&writer, a, NULL);
    bc_write_call_op(&writer, b, NULL);

    test_assert(writer.data->operationCount == 6);
    test_assert(writer.data->operations[0].type == OP_CALL);
    test_assert(writer.data->operations[1].type == OP_INPUT_GLOBAL);
    test_assert(writer.data->operations[2].type == OP_INPUT_GLOBAL);
    test_assert(writer.data->operations[3].type == OP_CALL);
    test_assert(writer.data->operations[4].type == OP_INPUT_GLOBAL);
    test_assert(writer.data->operations[5].type == OP_INPUT_GLOBAL);

    OpCall* op0 = (OpCall*) &writer.data->operations[0];
    test_assert(op0->term == a);
    OpCall* op1 = (OpCall*) &writer.data->operations[3];
    test_assert(op1->term == b);
}

void test_no_instructions_for_value()
{
    Branch branch;
    branch.compile("a = 1");
    update_bytecode_for_branch(&branch);
    test_assert(branch.bytecode->operationCount == 1);
    test_assert(branch.bytecode->operations[0].type == OP_RETURN);
}

TaggedValue globalForInputOverride;

void input_override_for_test(void*, Term* term, Operation* op)
{
    if (term->name == "override_me")
        bc_write_global_input(op, &globalForInputOverride);
}

void test_input_override()
{
    Branch branch;
    branch.compile("override_me = 1");
    Term* sum = branch.compile("sum = add_i(override_me override_me)");

    // test run, no override
    EvalContext context;
    context.preserveLocals = true;
    evaluate_branch(&context, branch);
    test_equals(sum, "2");

    // now install an override
    BytecodeWriter writer;
    writer.inputOverride = input_override_for_test;
    set_int(&globalForInputOverride, 5);

    push_stack_frame(&context, &branch);
    bc_call(&writer, sum);
    bc_finish(&writer);
    evaluate_bytecode(&context, writer.data);
    copy_locals_to_terms(&context, branch);

    test_equals(sum, "10");
}

void test_jump_if()
{
    BytecodeWriter writer;
    EvalContext context;

    TaggedValue s;
    TaggedValue b;

    // Sanity check, write bytecode to call test_spy()
    bc_imaginary_call(&writer, get_global("test_spy"));
    bc_global_input(&writer, &s);
    bc_finish(&writer);

    set_string(&s, "1");
    testing_clear_spy();
    evaluate_bytecode(&context, writer.data);
    test_equals(testing_get_spy_results(), "['1']");

    // First test, use jump_if to avoid a call.
    bc_reset_writer(&writer);
    int jump = bc_jump_if(&writer);
    bc_global_input(&writer, &b);
    set_bool(&b, true);

    bc_imaginary_call(&writer, get_global("test_spy"));
    bc_global_input(&writer, &s);
    bc_jump_to_here(&writer, jump);
    bc_finish(&writer);

    testing_clear_spy();
    evaluate_bytecode(&context, writer.data);
    test_equals(testing_get_spy_results(), "[]");

    // Rerun bytecode, this time send 'false' to the jump_if instruction.
    set_bool(&b, false);
    set_string(&s, "2");
    testing_clear_spy();
    evaluate_bytecode(&context, writer.data);
    test_equals(testing_get_spy_results(), "['2']");
}

void register_tests()
{
    REGISTER_TEST_CASE(bytecode_tests::test_simple_write);
    REGISTER_TEST_CASE(bytecode_tests::test_no_instructions_for_value);
    REGISTER_TEST_CASE(bytecode_tests::test_input_override);
    REGISTER_TEST_CASE(bytecode_tests::test_jump_if);
}

}
} // namespace circa
