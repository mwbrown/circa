// Copyright 2008 Paul Hodge

#include "tests/common.h"
#include "common_headers.h"
#include "branch.h"
#include "builtins.h"
#include "operations.h"
#include "parser.h"
#include "type.h"

namespace circa {
namespace type_tests {

void compound_types()
{
    Branch branch;

    Term* type1 = eval_statement(branch, "type1 = create-compound-type('type1)");
    test_assert(type1 != NULL);
    test_assert(is_type(type1));

    type1 = eval_statement(branch, "compound-type-append-field(@type1, int, 'myint)");
    test_assert(is_type(type1));
    test_assert(as_type(type1)->fields.size() == 1);
    test_assert(as_type(type1)->fields[0].name == "myint");
    test_assert(as_type(type1)->fields[0].type == INT_TYPE);

    type1 = eval_statement(branch, "compound-type-append-field(@type1, string, 'astr)");
    test_assert(is_type(type1));
    test_assert(as_type(type1)->fields.size() == 2);
    test_assert(as_type(type1)->fields[1].name == "astr");
    test_assert(as_type(type1)->fields[1].type == STRING_TYPE);

    Term* inst1 = eval_statement(branch, "inst1 = type1()");

    // test get-field function
    Term* inst1_myint = eval_statement(branch, "get-field(inst1, 'myint)");
    test_assert(inst1_myint != NULL);
    //test_assert(is_int(inst1_myint));

    Term* inst1_astr = eval_statement(branch, "get-field(inst1, 'astr)");
    test_assert(inst1_myint != NULL);
    //test_assert(is_string(inst1_myint));
    
    // test Term::field
    test_assert(inst1->field("myint") == inst1_myint);

    inst1->field("myint")->asInt() = 5;

    Term* inst2 = eval_statement(branch, "inst2 = type1()");

    test_assert(inst1->field("myint") != inst2->field("myint"));
    inst2->field("myint")->asInt() = 7;
    test_assert(inst1->field("myint")->asInt() == 5);
}

} // namespace type_tests

void register_type_tests()
{
    REGISTER_TEST_CASE(type_tests::compound_types);
}

} // namespace circa
