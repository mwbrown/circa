// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "circa.h"
#include "../importing.h"
#include "../importing_macros.h"

#include "types/ref.h"

namespace circa {
namespace vectorized_functions {

    Type* specializeType_vs(Term* caller)
    {
        #if 0
        Term* lhsType = caller->input(0)->type;
        if (is_list_based_type(unbox_type(lhsType)))
            return lhsType;
        #endif
        return &LIST_T;
    }

    void write_bytecode(Term* term, BytecodeWriter* writer)
    {
        Branch* contents = nested_contents(term);
        bc_call(writer, contents->get(0));
    }

    void post_input_change_vs(Term* term)
    {
        Branch* branch = nested_contents(term);
        clear_branch(branch);

        Term* forTerm = apply(branch, FOR_FUNC, TermList(term->input(0)));
        setup_for_loop_pre_code(forTerm);
        Term* iterator = for_loop_get_iterator(forTerm);

        Term* function = as_ref(&get_function_attrs(term->function)->parameter);

        apply(nested_contents(forTerm), function, TermList(iterator, term->input(1)));

        setup_for_loop_post_code(forTerm);
    }

    void post_input_change_vv(Term* term)
    {
        Branch* branch = nested_contents(term);
        clear_branch(branch);

        Term* forTerm = apply(branch, FOR_FUNC, TermList(term->input(0)));
        setup_for_loop_pre_code(forTerm);
        Term* iterator = for_loop_get_iterator(forTerm);

        Term* function = as_ref(&get_function_attrs(term->function)->parameter);

        apply(nested_contents(forTerm), function, TermList(iterator, term->input(1)));

        setup_for_loop_post_code(forTerm);
    }

    Type* specializeType_vv(Term* caller)
    {
        Type* lhsType = caller->input(0)->type;
        if (is_list_based_type(lhsType))
            return lhsType;
        return &LIST_T;
    }

    void setup(Branch* kernel)
    {
        Term* vs = import_function(kernel, NULL, "vectorize_vs(List,any) -> List");
        get_function_attrs(vs)->specializeType = specializeType_vs;
        get_function_attrs(vs)->postInputChange = post_input_change_vs;
        get_function_attrs(vs)->writeBytecode = write_bytecode;
        get_function_attrs(vs)->createsStackFrame = false;

        Term* vv = import_function(kernel, NULL, "vectorize_vv(List,List) -> List");
        get_function_attrs(vv)->specializeType = specializeType_vv;
        get_function_attrs(vv)->postInputChange = post_input_change_vv;
    }
}
} // namespace circa
