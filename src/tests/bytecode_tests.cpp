// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "bytecode.h"
#include "testing.h"

namespace circa {
namespace bytecode_tests {

void test_simple_write()
{
    Branch branch;
    Term* a = branch.compile("a = 1");
    Term* b = branch.compile("b = 1");
    BytecodeWriter writer;

    bytecode_call(&writer, a, NULL);
    bytecode_call(&writer, b, NULL);

    test_assert(writer.data->operationCount == 2);
    test_assert(writer.data->operations[0].type == OP_CALL);
    test_assert(writer.data->operations[1].type == OP_CALL);

    OpCall* op0 = (OpCall*) &writer.data->operations[0];
    test_assert(op0->term == a);
    OpCall* op1 = (OpCall*) &writer.data->operations[1];
    test_assert(op1->term == b);
}

void register_tests()
{
    REGISTER_TEST_CASE(bytecode_tests::test_simple_write);
}

}
} // namespace circa
