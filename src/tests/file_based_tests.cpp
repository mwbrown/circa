// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>
#include "filesystem_dummy.h"

namespace circa {
namespace file_based_tests {

void test_the_test()
{
    // Make sure that the file IO redirection thingy is working.
    FakeFileSystem files;

    files["a.txt"] = "hello";
    test_assert(read_text_file_as_str("a.txt") == "hello");

    write_text_file("a.txt", "goodbye");
    test_assert(read_text_file_as_str("a.txt") == "goodbye");
    test_assert(files["a.txt"] == "goodbye");

    // Read directory
    List fileList;
    read_directory_as_list(".", &fileList);
    test_equals(&fileList, "['a.txt']");

    files["b.txt"] = "hi";
    fileList.clear();
    read_directory_as_list(".", &fileList);
    test_equals(&fileList, "['a.txt', 'b.txt']");
}

void test_include_function()
{
    EvalContext context;
    FakeFileSystem files;

    files["file.ca"] = "a = 1";

    // Basic test, load our fake file.
    Branch branch;
    Term* incl = branch.compile("include('file.ca')");

    Branch& included = nested_contents(incl);

    // Make sure that the included file is loaded, even though the term wasn't evaluated.
    test_assert(included.length() > 0);

    test_assert(included["a"]->asInt() == 1);
    test_equals(get_branch_source_filename(included), "file.ca");

    // Next, modify the file and reload.
    files["file.ca"] = "b = 2";
    files.last_modified("file.ca")++;

    evaluate_save_locals(&context, branch);

    test_assert(!included.contains("a"));
    test_assert(included["b"]->asInt() == 2);

    // Modify the file but don't modify the last_modified time, make sure that
    // it doesn't reload this time.
    files["file.ca"] = "c = 3";
    evaluate_save_locals(&context, branch);
    test_assert(!included.contains("a"));
    test_assert(!included.contains("c"));
    test_assert(included["b"]->asInt() == 2);
}

void test_include_static_error_after_reload()
{
    FakeFileSystem files;
    Branch branch;

    files["file.ca"] = "add(1,1)";
    branch.compile("include('file.ca')");

    EvalContext result;
    evaluate_save_locals(&result, branch);

    test_assert(!result.errorOccurred);

    files["file.ca"] = "add(what what)";
    files.last_modified("file.ca")++;

    evaluate_save_locals(&result, branch);
    test_assert(result.errorOccurred);
}

void test_file_changed()
{
    Branch branch;
    FakeFileSystem files;
    files["x"] = "1";
    files["y"] = "2";

    Term* filename = branch.compile("filename = 'x'");
    Term* changed = branch.compile("file_changed(filename)");

    // First time through should always return true
    EvalContext context;
    evaluate_save_locals(&context, branch);
    test_assert(as_bool(changed));

    // Subsequent call should return false
    evaluate_save_locals(&context, branch);
    //dump(branch);
    //std::cout << context.state.toString();
    test_assert(!as_bool(changed));
    evaluate_save_locals(&context, branch);
    test_assert(!as_bool(changed));

    // Change the modified time
    files.last_modified("x")++;
    evaluate_save_locals(&context, branch);
    test_assert(as_bool(changed));
    evaluate_save_locals(&context, branch);
    test_assert(!as_bool(changed));

    // Change the filename
    set_string(filename, "y");
    evaluate_save_locals(&context, branch);
    test_assert(as_bool(changed));
    evaluate_save_locals(&context, branch);
    test_assert(!as_bool(changed));
}

void test_include_namespace()
{
    Branch branch;
    FakeFileSystem files;
    files["file"] = "namespace ns { a = 5 }";

    branch.compile("include('file')");
    Term* a = branch.compile("ns:a");
    evaluate_save_locals(branch);

    test_assert(branch);
    test_assert(as_int(a) == 5);
}

void test_include_with_error()
{
    // If we include a file, and this file has a static error, then make sure
    // that this error is not hidden. In previous code, the include() function
    // would reject the errorred code, and then we'd never know an error happened.

    Branch branch;
    FakeFileSystem files;
    files["file"] = "eyjafjallajokull";

    branch.compile("include('file')");
    evaluate_save_locals(branch);
    test_assert(has_static_errors(branch));
}

void test_include_from_expression()
{
    Branch branch;
    FakeFileSystem files;
    files["a"] = "x = 1";

    branch.compile("name = cond(true,'a','b')");
    branch.compile("include(name)");

    evaluate_save_locals(branch);
}

void test_include_with_state()
{
    Branch branch;
    branch.compile("state a = 1");
    FakeFileSystem files;
    files["file"] = "state b = 2; b = 3";
    branch.compile("include('file')");

    EvalContext context;
    evaluate_save_locals(&context, branch);
    test_equals(&context.state, "{_include: {b: 3}, a: 1}");
}

void test_call_function_from_included_file()
{
    FakeFileSystem files;
    files["file"] = "def hi() -> int { return 1 }";
    Branch branch;
    Term* includeCall = branch.compile("include('file')");
    Term* hiCall = branch.compile("hi()");

    EvalContext context;
    evaluate_save_locals(&context, branch);

    files.last_modified("file")++;
    evaluate_minimum(&context, includeCall, NULL);

    evaluate_save_locals(&context, branch);

    test_assert(hiCall->function != NULL);
}

void load_nonexistant_file()
{
    Branch branch;
    FakeFileSystem files;
    load_script(&branch, "a");
    test_assert(has_static_errors(branch));

    files.set("a", "x = 1");
    clear_branch(&branch);
    load_script(&branch, "a");
    test_assert(!has_static_errors(branch));

    // Also try using include() on a nonexistant file
    clear_branch(&branch);
    branch.compile("include('b')");
    evaluate_save_locals(branch);
    test_assert(has_static_errors(branch));
}

void test_include_script()
{
    // test the include_script() func
    Branch branch;
    FakeFileSystem files;
    files.set("a", "x = 1");

    include_script(&branch, "a");
    branch.compile("y = x");
    evaluate_save_locals(branch);
    test_equals(branch["y"], "1");

    files.set("a", "x = 2");
    evaluate_save_locals(branch);
    test_equals(branch["y"], "2");
}

void register_tests()
{
    REGISTER_TEST_CASE(file_based_tests::test_the_test);
    REGISTER_TEST_CASE(file_based_tests::test_include_function);
    REGISTER_TEST_CASE(file_based_tests::test_include_static_error_after_reload);
    REGISTER_TEST_CASE(file_based_tests::test_file_changed);
    REGISTER_TEST_CASE(file_based_tests::test_include_namespace);
    REGISTER_TEST_CASE(file_based_tests::test_include_with_error);
    REGISTER_TEST_CASE(file_based_tests::test_include_from_expression);
    REGISTER_TEST_CASE(file_based_tests::test_include_with_state);
    REGISTER_TEST_CASE(file_based_tests::test_call_function_from_included_file);
    REGISTER_TEST_CASE(file_based_tests::load_nonexistant_file);
    REGISTER_TEST_CASE(file_based_tests::test_include_script);
}

} // namespace file_based_tests

} // namespace circa
