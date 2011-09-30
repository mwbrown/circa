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
        append_phrase(source, get_for_loop_iterator(term)->name.c_str(),
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

    CA_FUNCTION(evaluate_loop_index)
    {
        // This call is evaluated once at the start of the loop.
        set_int(OUTPUT, 0);
    }

    CA_FUNCTION(loop_prepare_output)
    {
        List* inputList = as_list(INPUT(0));
        List* output = set_list(OUTPUT, inputList->length());
    }

    CA_FUNCTION(evaluate_break)
    {
        for_loop_break(CONTEXT);
    }
    CA_FUNCTION(evaluate_continue)
    {
        for_loop_continue(CONTEXT);
    }
    CA_FUNCTION(evaluate_discard)
    {
        CONTEXT->forLoopContext.discard = true;
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
        FOR_FUNC = import_function(kernel, evaluate_for_loop, "for(Indexable) -> List");
        get_function_attrs(FOR_FUNC)->formatSource = formatSource;
        get_function_attrs(FOR_FUNC)->getOutputCount = getOutputCount;
        get_function_attrs(FOR_FUNC)->getOutputName = getOutputName;
        get_function_attrs(FOR_FUNC)->getOutputType = getOutputType;
        //get_function_attrs(FOR_FUNC)->writeBytecode = for_block_write_bytecode;
        //get_function_attrs(FOR_FUNC)->writeNestedBytecode = for_block_write_bytecode_contents;
        get_function_attrs(FOR_FUNC)->beginBranch = for_loop_begin_branch;
        get_function_attrs(FOR_FUNC)->finishBranch = for_loop_finish_iteration;

        LOOP_INDEX_FUNC = import_function(kernel, NULL, "loop_index() -> int");

        import_function(kernel, loop_prepare_output, "loop_prepare_output(Indexable)->List");

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
