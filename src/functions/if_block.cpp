// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "circa.h"

#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace if_block_function {

    void formatSource(StyledSource* source, Term* term)
    {
        Branch* contents = nested_contents(term);

        for (int branch_index=0; branch_index < contents->length(); branch_index++) {
            Term* branch_term = contents->get(branch_index);

            if (is_hidden(branch_term))
                continue;

            append_phrase(source,
                    branch_term->stringPropOptional("syntax:preWhitespace", ""),
                    branch_term, token::WHITESPACE);

            if (branch_index == 0) {
                append_phrase(source, "if ", branch_term, phrase_type::KEYWORD);
                format_source_for_input(source, branch_term, 0);
            } else if (branch_index < (contents->length()-2)) {
                append_phrase(source, "elif ", branch_term, phrase_type::KEYWORD);
                format_source_for_input(source, branch_term, 0);
            }
            else
                append_phrase(source, "else", branch_term, phrase_type::UNDEFINED);

            format_branch_source(source, nested_contents(branch_term), branch_term);
        }
    }

    int getOutputCount(Term* term)
    {
        Branch* contents = nested_contents(term);

        // check if term is still being initialized:
        if (contents->length() == 0)
            return 1;

        Branch* outerRebinds = contents->getFromEnd(0)->contents();
        return outerRebinds->length() + 1;
    }

    const char* getOutputName(Term* term, int outputIndex)
    {
        Branch* contents = nested_contents(term);

        // check if term is still being initialized:
        if (contents->length() == 0)
            return "";

        Branch* outerRebinds = contents->getFromEnd(0)->contents();
        return outerRebinds->get(outputIndex - 1)->name.c_str();
    }

    Type* getOutputType(Term* term, int outputIndex)
    {
        if (outputIndex == 0)
            return &VOID_T;

        Branch* contents = nested_contents(term);

        // check if term is still being initialized:
        if (contents->length() == 0)
            return &ANY_T;

        Branch* outerRebinds = contents->getFromEnd(0)->contents();
        return outerRebinds->get(outputIndex - 1)->type;
    }

    Type* joinFunc_specializeType(Term* term)
    {
        if (term->input(0) == NULL || term->input(1) == NULL)
            return &ANY_T;
        List types;
        set_type_list(&types, get_type_of_input(term, 0), get_type_of_input(term, 1));
        return find_common_type(&types);
    }

    CA_FUNCTION(if_block_unpack_state)
    {
#if 0 // FIXME
        Term* caller = CALLER;
        EvalContext* context = CONTEXT;

        push_scope_state(context);
        Dict* prevScope = get_scope_state(context, 1);

        TaggedValue* stateEntry = prevScope->get(get_unique_name(caller));

        if (stateEntry == NULL)
            return;

        List* ifBlockState = List::lazyCast(stateEntry);
        int caseIndex = get_int_input(CONTEXT, CALL_OPERATION, 1);

        const bool resetStateForUnusedBranches = true;

        if (ifBlockState->length() <= caseIndex) {
            if (resetStateForUnusedBranches) set_null(stateEntry);
            return;
        }

        TaggedValue* stateItem = ifBlockState->get(caseIndex);
        if (!is_dict(stateItem))
            set_dict(stateItem);
        swap(stateItem, get_scope_state(context, 0));
        set_null(stateItem);

        if (resetStateForUnusedBranches)
            set_null(stateEntry);
#endif
    }

    CA_FUNCTION(if_block_pack_state)
    {
#if 0 // FIXME
        EvalContext* context = CONTEXT;
        Term* caller = CALLER;

        Dict* prevScope = get_scope_state(context, 1);
        List* stateEntry = List::lazyCast(prevScope->insert(get_unique_name(caller)));

        int caseIndex = get_int_input(CONTEXT, CALL_OPERATION, 1);
        if (stateEntry->length() <= caseIndex)
            stateEntry->resize(caseIndex + 1);

        swap(get_scope_state(context, 0), stateEntry->get(caseIndex));
        pop_scope_state(context);
#endif
    }

    void setup(Branch* kernel)
    {
        IF_BLOCK_FUNC = import_function(kernel, NULL, "if_block() -> any");
        get_function_attrs(IF_BLOCK_FUNC)->formatSource = formatSource;
        get_function_attrs(IF_BLOCK_FUNC)->getOutputCount = getOutputCount;
        get_function_attrs(IF_BLOCK_FUNC)->getOutputName = getOutputName;
        get_function_attrs(IF_BLOCK_FUNC)->getOutputType = getOutputType;
        get_function_attrs(IF_BLOCK_FUNC)->writeBytecode = if_block_write_calling_bytecode;

        JOIN_FUNC = import_function(kernel, NULL, "join(any...) -> any");
        get_function_attrs(JOIN_FUNC)->specializeType = joinFunc_specializeType;

        IF_BLOCK_UNPACK_STATE_FUNC = import_function(kernel, if_block_unpack_state,
                "if_block_unpack_state(int caseIndex)");
        IF_BLOCK_PACK_STATE_FUNC = import_function(kernel, if_block_pack_state,
                "if_block_pack_state(int caseIndex)");
    }
}
}
