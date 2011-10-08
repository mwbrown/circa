// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "circa.h"

#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace for_function {

    void format_heading(StyledSource* source, Term* term)
    {
        format_name_binding(source, term);
        append_phrase(source, "for ", term, phrase_type::KEYWORD);
        append_phrase(source, for_loop_get_iterator(term)->name.c_str(),
                term, phrase_type::UNDEFINED);
        append_phrase(source, " in ", term, phrase_type::KEYWORD);
        format_source_for_input(source, term, 0);
    }

    void formatSource(StyledSource* source, Term* term)
    {
        format_heading(source, term);
        format_branch_source(source, nested_contents(term), term);
        append_phrase(source, term->stringPropOptional("syntax:whitespaceBeforeEnd", ""),
            term, token::WHITESPACE);
    }

    int getOutputCount(Term* term)
    {
        Branch* contents = nested_contents(term);
        
        // Check if we're still building
        if (contents->length() == 0)
            return 1;

        Branch* outerRebinds = nested_contents(contents->get("#outer_rebinds"));
        return 1 + outerRebinds->length();
    }

    const char* getOutputName(Term* term, int outputIndex)
    {
        Branch* contents = nested_contents(term);

        // Check if we're still building
        if (contents->length() == 0)
            return "";

        Branch* outerRebinds = nested_contents(contents->get("#outer_rebinds"));
        return outerRebinds->get(outputIndex - 1)->name.c_str();
    }

    Type* getOutputType(Term* term, int outputIndex)
    {
        Branch* contents = nested_contents(term);

        // Check if we're still building
        if (contents->length() == 0)
            return &ANY_T;

        Branch* outerRebinds = nested_contents(contents->get("#outer_rebinds"));
        return outerRebinds->get(outputIndex - 1)->type;
    }

    void get_index__write_bytecode(BytecodeWriter* writer, Term* term)
    {
        Term* enclosingLoop = find_enclosing_for_loop(term);
        ca_assert(enclosingLoop != NULL);
        int distance = get_frame_distance(term, enclosingLoop) - 1;
        bc_copy(writer);
        bc_local_input(writer, term);
        bc_local_input(writer, distance, 0);
    }

    CA_FUNCTION(evaluate_break)
    {
    }
    CA_FUNCTION(evaluate_continue)
    {
    }
    CA_FUNCTION(evaluate_discard)
    {
    }
    void break_formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "break", term, phrase_type::KEYWORD);
    }
    void continue_formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "continue", term, phrase_type::KEYWORD);
    }
    void discard_formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "discard", term, phrase_type::KEYWORD);
    }

    void setup(Branch* kernel)
    {
        FOR_FUNC = import_function(kernel, NULL, "for(Indexable) -> List");
        get_function_attrs(FOR_FUNC)->formatSource = formatSource;
        get_function_attrs(FOR_FUNC)->getOutputCount = getOutputCount;
        get_function_attrs(FOR_FUNC)->getOutputName = getOutputName;
        get_function_attrs(FOR_FUNC)->getOutputType = getOutputType;
        get_function_attrs(FOR_FUNC)->writeBytecode = for_block_write_bytecode;
        get_function_attrs(FOR_FUNC)->writeNestedBytecode = for_block_write_bytecode_contents;
        get_function_attrs(FOR_FUNC)->createsStackFrame = true;

        Term* indexFunc = import_function(kernel, NULL, "index() -> int");
        get_function_attrs(indexFunc)->writeBytecode = get_index__write_bytecode;
        register_builtin_func(INDEX_FUNC, indexFunc);

        DISCARD_FUNC = import_function(kernel, evaluate_discard, "discard(any)");
        get_function_attrs(DISCARD_FUNC)->formatSource = discard_formatSource;
        hide_from_docs(DISCARD_FUNC);

        BREAK_FUNC = import_function(kernel, evaluate_break, "break()");
        get_function_attrs(BREAK_FUNC)->formatSource = break_formatSource;
        hide_from_docs(BREAK_FUNC);

        CONTINUE_FUNC = import_function(kernel, evaluate_continue, "continue()");
        get_function_attrs(CONTINUE_FUNC)->formatSource = continue_formatSource;
        hide_from_docs(CONTINUE_FUNC);
    }
}
} // namespace circa
