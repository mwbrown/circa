// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"
#include "heap_debugging.h"

namespace circa {
namespace builtin_type_tests {

void test_reference()
{
    Branch branch;

    Term* r1 = create_value(branch, &REF_T);
    Term* a = create_value(branch, &INT_T);
    Term* r2 = create_value(branch, &REF_T);

    set_ref(r1, a);

    test_assert(as_ref(r1) == a);

    copy(r1, r2);

    test_assert(as_ref(r2) == a);
}

void reference_type_deletion_bug()
{
    // There used to be a bug where deleting a reference term would delete
    // the thing it was pointed to.
    Branch *branch = new Branch();

    Term* myref = create_value(*branch, &REF_T);

    set_ref(myref, INT_TYPE);

    delete branch;

    assert_valid_term(INT_TYPE);
    test_assert(INT_TYPE->type != NULL);
}

void test_set()
{
#if 0 // TEST_DISABLED - Need a new evaluate_range
    Branch branch;

    Term* s = branch.eval("s = Set()");

    test_assert(s->numElements() == 0);

    s = branch.eval("s.add(1)");

    test_assert(s->numElements() == 1);
    test_assert(s->getIndex(0)->asInt() == 1);

    s = branch.eval("s.add(1)");
    test_assert(s->numElements() == 1);

    s = branch.eval("s.add(2)");
    test_assert(branch);
    test_assert(s->numElements() == 2);

    s = branch.eval("s.remove(1)");
    test_assert(s->numElements() == 1);
    test_assert(s->getIndex(0)->asInt() == 2);

    // check that things are copied by value
    Term* val = branch.eval("val = 5");
    s = branch.eval("s.add(val)");

    test_assert(s->getIndex(1)->asInt() == 5);
    set_int(val, 6);
    test_assert(s->getIndex(1)->asInt() == 5);
#endif
}

void test_list()
{
    Branch branch;

    Term* l = branch.compile("l = List()");

    evaluate_branch(branch);
    test_assert(is_list(l));
    test_assert(l->numElements() == 0);

    l = branch.compile("l.append(2)");
    evaluate_branch(branch);
    test_assert(l->numElements() == 1);
    test_assert(l->getIndex(0)->asInt() == 2);
}

void test_namespace()
{
    TermNamespace nspace;
    Term *term = alloc_term();

    nspace.bind(term, "a");
    test_assert(nspace.contains("a"));
    test_assert(nspace["a"] == term);

    Term *term2 = alloc_term();
    TermMap remap;
    remap[term] = term2;
    nspace.remapPointers(remap);
    test_assert(nspace["a"] == term2);

    test_assert(nspace.contains("a"));
    remap[term2] = NULL;
    nspace.remapPointers(remap);
    test_assert(!nspace.contains("a"));

    dealloc_term(term);
    dealloc_term(term2);
}

void register_tests()
{
    REGISTER_TEST_CASE(builtin_type_tests::test_reference);
    REGISTER_TEST_CASE(builtin_type_tests::reference_type_deletion_bug);
    REGISTER_TEST_CASE(builtin_type_tests::test_set);
    REGISTER_TEST_CASE(builtin_type_tests::test_list);
    REGISTER_TEST_CASE(builtin_type_tests::test_namespace);
}

} // namespace builtin_type_tests

} // namespace circa
