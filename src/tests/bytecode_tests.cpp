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

    bytecode_call(&writer, a, NULL);
    bytecode_call(&writer, b, NULL);

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
        bytecode_write_global_input(op, &globalForInputOverride);
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
    write_bytecode_for_term(&writer, sum);
    bytecode_return(&writer);
    evaluate_bytecode(&context, writer.data);
    copy_locals_to_terms(&context, branch);

    test_equals(sum, "10");
}

void register_tests()
{
    REGISTER_TEST_CASE(bytecode_tests::test_simple_write);
    REGISTER_TEST_CASE(bytecode_tests::test_no_instructions_for_value);
    REGISTER_TEST_CASE(bytecode_tests::test_input_override);
}

}
} // namespace circa
