// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "kernel.h"
#include "building.h"
#include "interpreter.h"
#include "inspection.h"
#include "list.h"
#include "names.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"

namespace circa {

Type* find_common_type(caValue* list)
{
    if (list_length(list) == 0)
        return TYPES.any;

    // Check if every type in this list is the same.
    bool all_equal = true;
    for (int i=1; i < list_length(list); i++) {
        if (as_type(list_get(list,0)) != as_type(list_get(list,i))) {
            all_equal = false;
            break;
        }
    }

    if (all_equal)
        return as_type(list_get(list,0));

    // Special case, allow ints to go into floats
    bool all_are_ints_or_floats = true;
    for (int i=0; i < list_length(list); i++) {
        if ((as_type(list_get(list,i)) != TYPES.int_type)
                && (as_type(list_get(list,i)) != TYPES.float_type)) {
            all_are_ints_or_floats = false;
            break;
        }
    }

    if (all_are_ints_or_floats)
        return TYPES.float_type;

    // Another special case, if all types are lists then use List
    bool all_are_lists = true;
    for (int i=0; i < list_length(list); i++) {
        if (!is_list_based_type(as_type(list_get(list,i))))
            all_are_lists = false;
    }

    if (all_are_lists)
        return TYPES.list;

    // Otherwise give up
    return TYPES.any;
}

Type* find_common_type(Type* type1, Type* type2)
{
    if (type1 == NULL)
        return type2;

    if (type1 == type2)
        return type1;

    if (type1 == TYPES.any || type2 == TYPES.any)
        return TYPES.any;

    if (is_list_based_type(type1) && is_list_based_type(type2))
        return TYPES.list;

    return TYPES.any;
}

Type* find_common_type(Type* type1, Type* type2, Type* type3)
{
    List list;
    set_type_list(&list, type1, type2, type3);
    return find_common_type(&list);
}

Type* infer_type_of_get_index(Term* input)
{
    if (input == NULL)
        return TYPES.any;

    switch (list_get_parameter_type(&input->type->parameter)) {
    case sym_Untyped:
        return TYPES.any;
    case sym_UniformListType:
        return list_get_repeated_type_from_type(input->type);
    case sym_StructType:
    case sym_AnonStructType:
        return find_common_type(list_get_type_list_from_type(input->type));
    case sym_Invalid:
    default:
        return TYPES.any;
    }
}

Term* statically_infer_length_func(Block* block, Term* term)
{
    Term* input = term->input(0);
    if (input->function == FUNCS.copy)
        return statically_infer_length_func(block, input->input(0));

    if (input->function == FUNCS.list)
        return create_int(block, input->numInputs());

    if (input->function == FUNCS.list_append) {
        Term* leftLength = apply(block, FUNCS.length, TermList(input->input(0)));
        return apply(block, FUNCS.add, TermList(
                    statically_infer_length_func(block, leftLength),
                    create_int(block, 1)));
    }

    // Give up
    std::cout << "statically_infer_length_func didn't understand: "
        << input->function->name << std::endl;
    return create_symbol_value(block, sym_Unknown);
}

Term* statically_infer_result(Block* block, Term* term)
{
    if (term->function == FUNCS.length)
        return statically_infer_length_func(block, term);

#if 0
    Disabled, this approach might be flawed
    if (term->function == TYPE_FUNC)
        return statically_infer_type(block, term->input(0));
#endif

    // Function not recognized
    std::cout << "statically_infer_result didn't recognize: "
        << term->function->name << std::endl;
    return create_symbol_value(block, sym_Unknown);
}

void statically_infer_result(Term* term, caValue* result)
{
    Block scratch;

    Term* resultTerm = statically_infer_result(&scratch, term);

    if (is_value(resultTerm))
        copy(term_value(resultTerm), result);
    else {
        Stack context;
        evaluate_minimum(&context, resultTerm, result);
    }
}

Type* create_typed_unsized_list_type(Type* elementType)
{
    Type* type = create_type();
    list_t::setup_type(type);
    set_type(&type->parameter, elementType);
    std::string name = std::string("List<") + as_cstring(&elementType->name) + ">";
    set_string(&type->name, name.c_str());
    return type;
}

} // namespace circa
