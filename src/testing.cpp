// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"
#include "heap_debugging.h"

namespace circa {

std::vector<TestCase> gTestCases;

TestCase gCurrentTestCase;

// this function is defined in register_all_tests.cpp
void register_all_tests();

void post_test_sanity_check();

void test_assert_function(bool condition, int line, const char* file)
{
    if (!condition) {
        std::stringstream msg;
        msg << "Assert failure in " << file << ", line " << line;
        throw std::runtime_error(msg.str());
    }
}

void test_assert_function(Term* term, int line, const char* file)
{
    if (term == NULL) {
        std::stringstream msg;
        msg << "NULL term in " << file << ", line " << line;
        throw std::runtime_error(msg.str());
    }

    if (has_static_error(term)) {
        std::stringstream msg;
        msg << "Compile error on term " << global_id(term) << std::endl;
        print_static_error(term, msg);
        msg << std::endl;
        msg << "Occurred in " << file << ", line " << line << std::endl;
        throw std::runtime_error(msg.str());
    }

    if (is_bool(term) && !as_bool(term)) {
        std::stringstream msg;
        msg << "Assertion failed: " << get_term_source_text(term) << std::endl;
        msg << "Occurred in " << file << ", line " << line << std::endl;
        throw std::runtime_error(msg.str());

    }
}

void test_assert_function(Branch& branch, int line, const char* file)
{
    if (!branch_check_invariants_print_result(&branch, std::cout)) {
        declare_current_test_failed();
        throw std::runtime_error("");
    }

    List errors;
    check_for_static_errors(&errors, &branch);
    if (!errors.empty()) {
        std::stringstream msg;
        msg << "Branch has static errors at " << file << ", line " << line << std::endl;
        print_static_errors_formatted(&errors, msg);
        throw std::runtime_error(msg.str());
    }
}

void test_assert_function(EvalContext& context, int line, const char* file)
{
    if (context.errorOccurred) {
        std::stringstream msg;
        msg << "Runtime error at " << file << ", line " << line << std::endl;
        print_runtime_error_formatted(context, msg);
        throw std::runtime_error(msg.str());
    }
}

void test_fail_function(int line, const char* file)
{
    std::stringstream msg;
    msg << "Test fail in " << file << ", line " << line;
    throw std::runtime_error(msg.str());
}

void test_equals_function(TermList const& a, TermList const& b,
        const char* aText, const char* bText, int line, const char* file)
{
    std::stringstream msg;

    if (a.length() != b.length()) {
        msg << "List equality fail in " << file << ", line " << line << std::endl;
        msg << "  " << aText << " has " << a.length() << " items, ";
        msg << bText << " has " << b.length() << " items.";
        throw std::runtime_error(msg.str());
    }

    for (int i=0; i < a.length(); i++) {
        if (a[i] != b[i]) {
            msg << "List equality fail in " << file << ", line " << line << std::endl;
            msg << "  " << aText << " != " << bText << " (index " << i << " differs)";
            throw std::runtime_error(msg.str());
        }
    }
}

void test_equals_function(float a, float b,
        const char* aText, const char* bText,
        int line, const char* file)
{
    const float EPSILON = 0.0001f;

    if (fabs(a-b) > EPSILON) {
        std::stringstream msg;
        msg << "Equality fail in " << file << ", line " << line << std::endl;
        msg << aText << " [" << a << "] != " << bText << " [" << b << "]";
        throw std::runtime_error(msg.str());
    }
}

void test_equals_function(std::string a, std::string b,
        const char* aText, const char* bText,
        int line, const char* file)
{
    if (a != b) {
        std::stringstream msg;
        msg << "Failed assert: " << a << " != " << b;
        msg << ", in " << file << ", line " << line << std::endl;
        throw std::runtime_error(msg.str());
    }
}

void test_equals_function(TaggedValue* a, std::string b,
        const char* aText, const char* bText,
        int line, const char* file)
{
    return test_equals_function(is_string(a) ? as_string(a) : to_string(a),
            b, aText, bText, line, file);
}

void test_equals_function(TaggedValue* a, float b,
        const char* aText, const char* bText,
        int line, const char* file)
{
    return test_equals_function(a->toFloat(), b, aText, bText, line, file);
}

bool test_fail_on_static_error(Branch& branch)
{
    if (has_static_errors(&branch)) {
        std::cout << "Static error in " << get_current_test_name() << std::endl;
        print_static_errors_formatted(&branch, std::cout);
        std::cout << std::endl;
        declare_current_test_failed();
        return true;
    }
    return false;
}

bool test_fail_on_runtime_error(EvalContext& context)
{
    if (context.errorOccurred) {
        std::cout << "Runtime error in " << get_current_test_name() << std::endl;
        print_runtime_error_formatted(context, std::cout);
        std::cout << std::endl;
        declare_current_test_failed();
        return true;
    }
    return false;
}

bool run_test(TestCase& testCase, bool catch_exceptions)
{
    gCurrentTestCase = testCase;

    if (catch_exceptions) {
        try {
            gCurrentTestCase = testCase;
            testCase.execute();

            // the test code may declare itself failed
            bool result = !gCurrentTestCase.failed;

            post_test_sanity_check();
            return result;
        }
        catch (std::runtime_error const& err) {
            std::cout << "Error white running test case " << testCase.name << std::endl;
            std::cout << err.what() << std::endl;
            return false;
        }
    } else {
        testCase.execute();
    }

    post_test_sanity_check();

    return !gCurrentTestCase.failed;
}

bool run_tests(std::string const& searchStr)
{
    register_all_tests();

    int totalTestCount = 0;
    int successCount = 0;
    int failureCount = 0;
    std::vector<TestCase>::iterator it;
    for (it = gTestCases.begin(); it != gTestCases.end(); ++it) {
        if (it->name.find(searchStr) == std::string::npos)
            continue;
        totalTestCount++;
        std::cout << "Running " << it->name << std::endl;
        bool result = run_test(*it, false);
        if (result) successCount++;
        else {
            failureCount++;
            std::cout << "Test failed: " << it->name << std::endl;
        }
    }

    std::cout << "Ran " << totalTestCount << " tests. ";

    if (failureCount == 0) {
        std::cout << "All tests passed." << std::endl;
    } else {
        std::string tests = failureCount == 1 ? "test" : "tests";
        std::cout << failureCount << " " << tests << " failed." << std::endl;
    }

    return failureCount == 0;
}

bool run_all_tests()
{
    register_all_tests();

    int totalTestCount = 0;
    int successCount = 0;
    int failureCount = 0;
    std::vector<TestCase>::iterator it;
    for (it = gTestCases.begin(); it != gTestCases.end(); ++it) {
        totalTestCount++;
        bool result = run_test(*it, false);
        if (result) successCount++;
        else {
            failureCount++;
            std::cout << "Test failed: " << it->name << std::endl;
        }
    }

    std::cout << "Ran " << totalTestCount << " tests. ";

    if (failureCount == 0) {
        std::cout << "All tests passed." << std::endl;
    } else {
        std::string tests = failureCount == 1 ? "test" : "tests";
        std::cout << failureCount << " " << tests << " failed." << std::endl;
    }

    return failureCount == 0;
}

void post_test_sanity_check()
{
    // this once did something
}

std::vector<std::string> list_all_test_names()
{
    register_all_tests();

    std::vector<std::string> output;

    std::vector<TestCase>::iterator it;
    for (it = gTestCases.begin(); it != gTestCases.end(); ++it)
        output.push_back(it->name);

    return output;
}

std::string get_current_test_name()
{
    return gCurrentTestCase.name;
}

void declare_current_test_failed()
{
    gCurrentTestCase.failed = true;
}

bool current_test_has_failed()
{
    return gCurrentTestCase.failed;
}

void test_branch_as_assertions_list(Branch* branch, std::string const& contextStr)
{
    if (has_static_errors(branch)) {
        std::cout << "Static error " << contextStr << ":" << std::endl;
        print_static_errors_formatted(branch, std::cout);
        declare_current_test_failed();
        return;
    }

    std::stringstream checkInvariantsOutput;
    if (!branch_check_invariants_print_result(branch, checkInvariantsOutput)) {
        std::cout << "Failed invariant " << contextStr << std::endl;
        std::cout << checkInvariantsOutput.str() << std::endl;
        declare_current_test_failed();
        return;
    }

    EvalContext context;
    interpret_save_locals(&context, branch);

    if (context.errorOccurred) {
        std::cout << "Runtime error " << contextStr << std::endl;
        print_runtime_error_formatted(context, std::cout);
        declare_current_test_failed();
        return;
    }

    int boolean_statements_found = 0;
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (!is_statement(term))
            continue;

        if (!is_bool(term))
            continue;

        boolean_statements_found++;

        if (!as_bool(term)) {
            std::cout << "Assertion failed " << contextStr << std::endl;
            std::cout << "failed: " << get_term_source_text(term) << std::endl;
            declare_current_test_failed();
            return;
        }
    }

    if (boolean_statements_found == 0) {
        std::cout << "No boolean statements found " << contextStr << std::endl;
        declare_current_test_failed();
        return;
    }
}

void testing_clear_spy()
{
    internal_debug_function::spy_clear();
}
List* testing_get_spy_results()
{
    return internal_debug_function::spy_results();
}

}
