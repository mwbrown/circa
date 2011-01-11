// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace branch_iterator_tests {

void test_simple()
{
    Branch branch;

    Term* a = branch.compile("a = 1");
    Term* b = branch.compile("b = 2");

    test_assert(a);
    test_assert(b);

    BranchIterator it(branch);

    test_assert(it.current() == a);
    it.advance();
    test_assert(it.current() == b);
    it.advance();
    test_assert(it.finished());

    Term* sub = branch.compile("def func() c = 3 d = 4 end");

    it.reset(branch);

    test_assert(it.current() == a);
    it.advance();
    test_assert(it.current() == b);
    it.advance();
    test_assert(it.current() == sub);
    it.advance();
    test_assert(it.current() == sub->nestedContents[0]);
    it.advanceSkippingBranch(); // skip over 'attributes'
    test_assert(it.current()->name == "c");
    it.advance();
    test_assert(it.current()->name == "d");
    it.advance();
    test_assert(it.current()->function->name == "return");
    it.advance();
    test_assert(it.finished());
}

void register_tests()
{
    REGISTER_TEST_CASE(branch_iterator_tests::test_simple);
}

} // namespace branch_iterator_tests
} // namespace circa
