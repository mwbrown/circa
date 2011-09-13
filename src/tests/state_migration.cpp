// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace state_migration {

void test_migration(std::string sourceCode, std::string destinationCode,
    std::string assertionsCode)
{
    Branch source;
    parser::compile(source, parser::statement_list, sourceCode);

    if (has_static_errors(source)) {
        std::cout << "Static error in code: " << sourceCode << std::endl;
        print_static_errors_formatted(source, std::cout);
        std::cout << std::endl;
        declare_current_test_failed();
        return;
    }

    Branch destination;
    parser::compile(destination, parser::statement_list, destinationCode);

    if (has_static_errors(destination)) {
        std::cout << "Static error in code: " << destinationCode << std::endl;
        print_static_errors_formatted(destination, std::cout);
        std::cout << std::endl;
        declare_current_test_failed();
        return;
    }

    EvalContext context;
    context.preserveLocals = true;
    evaluate_branch(&context, source);

    if (context.errorOccurred) {
        std::cout << "Runtime error in " << get_current_test_name() << std::endl;
        print_runtime_error_formatted(context, std::cout);
        std::cout << std::endl;
        declare_current_test_failed();
        return;
    }

    Branch& assertions = create_branch(destination, "assertions");
    parser::compile(assertions, parser::statement_list, assertionsCode);

    evaluate_branch(&context, destination);

    if (context.errorOccurred) {
        std::cout << "In " << get_current_test_name() << std::endl;
        print_runtime_error_formatted(context, std::cout);
        std::cout << std::endl;
        declare_current_test_failed();
        return;
    }

    int boolean_statements_found = 0;
    for (int i=0; i < assertions.length(); i++) {
        if (!is_statement(assertions[i]))
            continue;

        TaggedValue* result = assertions[i];//get_local(&context, 0, assertions[i], 0);

        if (!is_bool(result))
            continue;

        boolean_statements_found++;

        if (!as_bool(result)) {
            std::cout << "In " << get_current_test_name() << std::endl;
            std::cout << "assertion failed: "
                << get_term_source_text(assertions[i]) << std::endl;
            std::cout << "Source:" << std::endl;
            print_branch(std::cout, source);
            std::cout << "Destination:" << std::endl;
            print_branch(std::cout, destination);
            std::cout << "State:" << std::endl;
            std::cout << context.state.toString() << std::endl;
            declare_current_test_failed();
            return;
        }
    }

    if (boolean_statements_found == 0 && assertions.length() > 0) {
        std::cout << "In " << get_current_test_name() << std::endl;
        std::cout << "warn: no boolean statements found in: " << assertionsCode << std::endl;
    }
}

void migrate_simple()
{
    test_migration("state i = 5", "state i = 4", "i == 5");
}

void migrate_across_user_defined_types()
{
    #if 0
    TEST_DISABLED

    // Pre-test work, make sure that 'state T t = [1]' works
    Branch branch;
    Term* typeT = branch.compile("type T { int x }");
    TaggedValue* x = branch.eval("x = [1]");
    test_assert(cast_possible(x, unbox_type(typeT)));

    // Type T is defined the same way
    test_migration("type T { int x } \n state T t = [1]",
        "type T { int x } \n state T t = [2]",
        "t.x == 1");

    // Type T has the same data types, but with a different field name
    test_migration("type T { int x } \n state T t = [1]",
        "type T { int y } \n state T t = [2]",
        "t.y == 1");
    #endif
}

void dont_migrate_across_different_types()
{
    //test_migration("state int i; i = 5", "state number i", "i == 0");
    test_migration("state Point p; p = [3 3]", "state Rect p", "p == [.0 .0 .0 .0]");

    test_migration("def f1()->int { state int i; return(i) }"
                   "def f2(int i) { state number n } f1()",
                   "def f1()->int { state int i; return(i) }"
                   "def f2(int i) { state number n } f2(f1())", "");
}

void migrate_complex_types()
{
    test_migration("state List asteroids = []; asteroids = [[[3.0 2.9] [[true false] 1 0 2]]]",
            "state List asteroids = []",
            "asteroids == [[[3.0 2.9] [[true false] 1 0 2]]]");
}

void migrate_namespace()
{
    return; // TEST_DISABLED
    test_migration("namespace ns { state s = 1.0 }",
            "namespace ns { state s }",
            "ns:s == 1.0");
}

void migrate_misc()
{
    // These tests don't have a specific focus

    #if 0
    TEST_DISABLED

    test_migration("type Point { number x, number y }\n"
                   "def get_ship_start_point() -> Point { return([50,50]) }"
                   "type Ship { Point loc, Point momentum, number facing }\n"
                   "state Ship ship = [get_ship_start_point() [0 0] 0]\n"
                   "ship = [[5 5] [1 1] 1]",

                   "type Point { number x, number y }\n"
                   "def get_ship_start_point() -> Point { return([50,50]) }"
                   "type Ship { Point loc, Point momentum, number facing }\n"
                   "state Ship ship = [get_ship_start_point() [0 0] 0]\n",

                   "ship.loc == [5.0 5.0], ship.momentum == [1.0 1.0], ship.facing == 1.0");
    #endif
}

void register_tests()
{
    REGISTER_TEST_CASE(state_migration::migrate_simple);
    REGISTER_TEST_CASE(state_migration::migrate_across_user_defined_types);
    REGISTER_TEST_CASE(state_migration::dont_migrate_across_different_types);
    REGISTER_TEST_CASE(state_migration::migrate_complex_types);
    REGISTER_TEST_CASE(state_migration::migrate_namespace);
    REGISTER_TEST_CASE(state_migration::migrate_misc);
}

} // namespace migration_snippets
} // namespace circa
