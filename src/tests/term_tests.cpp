// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace term_tests {

void duplicate_nested_contents()
{
    Branch branch;
    Term* a = branch.compile("a = branch()");

    Term* x = create_int(nested_contents(a), 5);

    // Try duplicating branch
    Branch branch2;
    duplicate_branch(branch, branch2);

    test_assert(branch2.length() == 1);
    test_assert(branch2[0]->name == "a");
    test_assert(branch2[0]->contents().length() == 1);
    test_assert(branch2[0]->contents()[0]->asInt() == 5);

    branch2.clear();

    // Now try duplicating, check that internal references are updated
    apply(nested_contents(a), "add", TermList(x, x));
    duplicate_branch(branch, branch2);

    test_assert(branch2.length() == 1);
    test_assert(branch2[0]->name == "a");
    test_assert(branch2[0]->contents().length() == 2);
    test_assert(branch2[0]->contents()[0]->asInt() == 5);
    test_assert(branch2[0]->contents()[1]->input(0) == branch2[0]->contents()[0]);
    test_assert(branch2[0]->contents()[1]->input(1) == branch2[0]->contents()[0]);
}

void register_tests()
{
    REGISTER_TEST_CASE(term_tests::duplicate_nested_contents);
}

} // namespace term_tests
} // namespace circa
