// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "heap_debugging.h"
#include "importing_macros.h"

namespace circa {
namespace tagged_value_tests {

namespace toy_refcounted_pool {
    const int pool_size = 5;
    int refcount[pool_size] = {0,};

    // helper functions for pool:
    bool nothing_allocated()
    {
        for (int i=0; i < pool_size; i++)
            if (refcount[i] != 0)
                return false;
        return true;
    }

    void initialize(Type*, TaggedValue* value)
    {
        for (int i=0; i < pool_size; i++) {
            if (refcount[i] == 0) {
                refcount[i]++;
                value->value_data.asint = i;
                return;
            }
        }
        ca_assert(false);
    }

    void release(Type*, TaggedValue* value)
    {
        int index = value->value_data.asint;
        ca_assert(refcount[index] > 0);
        refcount[index]--;
    }

    void copy(Type* type, TaggedValue* source, TaggedValue* dest)
    {
        change_type(dest, type);
        int prev = dest->value_data.asint;
        dest->value_data.asint = source->value_data.asint;
        refcount[dest->value_data.asint]++;
        refcount[prev]--;
    }
    void setup_type(Type* type)
    {
        type->initialize = initialize;
        type->release = release;
        type->copy = copy;
    }
}


void test_int_simple()
{
    TaggedValue v;
    set_int(&v, 4);

    test_assert(is_int(&v));
    test_assert(as_int(&v) == 4);

    Branch branch;
    Term* a = branch.compile("a = 1");
    test_assert(is_int(a));

    Term* b = branch.compile("b = a");
    test_assert(is_int(b));
}

void test_polymorphic()
{
    TaggedValue v;
    test_assert(!is_int(&v));
    test_assert(!is_float(&v));
    test_assert(!is_bool(&v));

    set_int(&v, 11);
    test_assert(is_int(&v));
    test_assert(!is_float(&v));
    test_assert(!is_bool(&v));

    set_float(&v, 2.0);
    test_assert(!is_int(&v));
    test_assert(is_float(&v));
    test_assert(!is_bool(&v));

    set_bool(&v, false);
    test_assert(!is_int(&v));
    test_assert(!is_float(&v));
    test_assert(is_bool(&v));
}

void test_term_value()
{
    Branch branch;
    Term* i = create_int(&branch, 5);
    test_assert(is_int(i));
    test_assert(is_int(i));

    EvalContext context;
    TaggedValue* a = branch.compile("a = [1 2 3]");
    TaggedValue* b = branch.compile("b = a");

    interpret_save_locals(&branch);
    test_assert(a->numElements() == 3);
    test_assert(a->getIndex(1)->asInt() == 2);

    test_assert(b->numElements() == 3);
    test_assert(b->getIndex(1)->asInt() == 2);
    
    Term* c = create_value(&branch, &INT_T);
    test_assert(is_int(c));
}

namespace subroutine_call_test_helper {
    CA_FUNCTION(assert_ints)
    {
        test_assert(is_int(INPUT(0)));
        set_int(OUTPUT, 0);
    }
}

void subroutine_call_test()
{
    Branch branch;
    import_function(&branch, subroutine_call_test_helper::assert_ints, "f(int) -> int");
    branch.eval("f(1)");
}

void test_reset()
{
    Branch branch;
    Term* a = create_int(&branch, 5);
    test_assert(as_int(a) == 5);
    reset(a);
    test_assert(as_int(a) == 0);
}

void test_constructor_syntax()
{
    Branch branch;
    Type* myType = Type::create();
    myType->name = "T";
    import_type(&branch, myType);
    TaggedValue* a = branch.eval("a = T()");
    test_assert(a->value_type == myType);
    test_assert(a->value_data.ptr == NULL);
    reset(a);
    test_assert(a->value_type == myType);
    test_assert(a->value_data.ptr == NULL);
}

namespace manual_memory_management_test {
    
    // toy memory pool:
    const int pool_size = 5;
    bool pool_allocated[pool_size] = {false,};

    // helper functions for pool:

    int pool_allocate()
    {
        for (int i=0; i < pool_size; i++) {
            if (!pool_allocated[i]) {
                pool_allocated[i] = true;
                return i;
            }
        }
        ca_assert(false);
        return -1;
    }

    void pool_deallocate(int index)
    {
        ca_assert(pool_allocated[index]);
        pool_allocated[index] = false;
    }

    // Type functions:
    void initialize(Type* type, TaggedValue* value)
    {
        value->value_data.asint = pool_allocate();
    }

    void release(Type*, TaggedValue* value)
    {
        pool_deallocate(value->value_data.asint);
    }

    void test()
    {
        Type* myType = Type::create();
        myType->initialize = initialize;
        myType->release = release;

        TaggedValue value;

        test_assert(is_null(&value));
        test_assert(!pool_allocated[0]);
        test_assert(!pool_allocated[1]);

        change_type(&value, myType);

        test_assert(pool_allocated[0]);
        test_assert(!pool_allocated[1]);

        set_null(&value);

        test_assert(!pool_allocated[0]);
        test_assert(!pool_allocated[1]);

        // scope 1:
        {
            TaggedValue scoped_value;
            change_type(&scoped_value, myType);
            test_assert(pool_allocated[0]);
        }
        test_assert(!pool_allocated[0]);

        // scope 2
        {
            TaggedValue scoped_value;
            change_type(&scoped_value, myType);
            test_assert(pool_allocated[0]);
            set_null(&scoped_value);
            test_assert(!pool_allocated[0]);
        }
        test_assert(!pool_allocated[0]);
    }
}

void refcount_test()
{
    Type* t = Type::create();
    toy_refcounted_pool::setup_type(t);

    {
        TaggedValue value(t);

        test_assert(toy_refcounted_pool::refcount[0] == 1);
    }

    test_assert(toy_refcounted_pool::nothing_allocated());

    {
        TaggedValue value1(t), value2(t);

        test_assert(toy_refcounted_pool::refcount[0] == 1);

        copy(&value1, &value2);

        test_assert(value1.value_data.asint == 0);
        test_assert(value2.value_data.asint == 0);

        test_assert(toy_refcounted_pool::refcount[0] == 2);
    }

    test_assert(toy_refcounted_pool::nothing_allocated());
}

void list_memory_management()
{
    List list;
    Type* t = Type::create();
    toy_refcounted_pool::setup_type(t);

    TaggedValue v(t);

    test_assert(toy_refcounted_pool::refcount[0] == 1);

    copy(&v, list.append());

    test_assert(toy_refcounted_pool::refcount[0] == 2);

    list.clear();

    test_assert(toy_refcounted_pool::refcount[0] == 1);

    swap(&v, list.append());

    test_assert(toy_refcounted_pool::refcount[0] == 1);

    list.clear();

    test_assert(toy_refcounted_pool::refcount[0] == 0);

    change_type(list.append(), t);
    change_type(list.append(), t);
    change_type(list.append(), t);

    test_assert(toy_refcounted_pool::refcount[0] == 1);
    test_assert(toy_refcounted_pool::refcount[1] == 1);
    test_assert(toy_refcounted_pool::refcount[2] == 1);

    set_null(list[1]);

    test_assert(toy_refcounted_pool::refcount[0] == 1);
    test_assert(toy_refcounted_pool::refcount[1] == 0);
    test_assert(toy_refcounted_pool::refcount[2] == 1);

    copy(list[2], list[0]);

    test_assert(toy_refcounted_pool::refcount[0] == 0);
    test_assert(toy_refcounted_pool::refcount[1] == 0);
    test_assert(toy_refcounted_pool::refcount[2] == 2);

    list.clear();

    test_assert(toy_refcounted_pool::nothing_allocated());
}

void reset_null()
{
    TaggedValue value;
    set_null(&value);
    reset(&value);
    debug_assert_valid_object(value.value_type, TYPE_OBJECT);
}

void resize_list_maintains_existing_data()
{
    TaggedValue outerTv;
    List* outer = set_list(&outerTv, 3);

    List* inner = set_list(outer->get(1), 4);
    TaggedValue* a = inner->get(1);
    set_int(a, 5);

    test_assert(outer->get(1)->getIndex(1) == a);

    outer->resize(5);

    test_assert(outer->get(1)->getIndex(1) == a);
}

void test_to_string_annotated()
{
    TaggedValue v;
    set_int(&v, 5);
    test_equals(to_string_annotated(&v), "int#5");

    List list;
    set_int(list.append(), 3);
    set_string(list.append(), "hi");
    test_equals(to_string_annotated(&list), "List#[int#3, string#'hi']");
}

void register_tests()
{
    REGISTER_TEST_CASE(tagged_value_tests::test_int_simple);
    REGISTER_TEST_CASE(tagged_value_tests::test_polymorphic);
    REGISTER_TEST_CASE(tagged_value_tests::test_term_value);
    REGISTER_TEST_CASE(tagged_value_tests::subroutine_call_test);
    REGISTER_TEST_CASE(tagged_value_tests::test_reset);
    REGISTER_TEST_CASE(tagged_value_tests::test_constructor_syntax);
    REGISTER_TEST_CASE(tagged_value_tests::manual_memory_management_test::test);
    REGISTER_TEST_CASE(tagged_value_tests::refcount_test);
    REGISTER_TEST_CASE(tagged_value_tests::list_memory_management);
    REGISTER_TEST_CASE(tagged_value_tests::reset_null);
    REGISTER_TEST_CASE(tagged_value_tests::resize_list_maintains_existing_data);
    REGISTER_TEST_CASE(tagged_value_tests::test_to_string_annotated);
}

}
} // namespace circa
