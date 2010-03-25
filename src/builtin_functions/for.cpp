// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace for_function {

    std::string get_heading_source(Term* term)
    {
        std::stringstream result;
        result << "for ";
        result << get_for_loop_iterator(term)->name;
        result << " in ";
        result << get_source_of_input(term, 1);
        return result.str();
    }

    void format_heading(RichSource* source, Term* term)
    {
        append_phrase(source, "for ", term, phrase_type::KEYWORD);
        append_phrase(source, get_for_loop_iterator(term)->name.c_str(),
                term, phrase_type::UNDEFINED);
        append_phrase(source, " in ", term, phrase_type::KEYWORD);
        append_source_for_input(source, term, 1);
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream result;
        result << get_heading_source(term);
        result << term->stringPropOptional("syntax:postHeadingWs", "\n");
        print_branch_source(result, term);
        result << term->stringPropOptional("syntax:whitespaceBeforeEnd", "");

        return result.str();
    }

    void formatSource(RichSource* source, Term* term)
    {
        format_heading(source, term);
        append_phrase(source, term->stringPropOptional("syntax:postHeadingWs", "\n"),
            term, token::WHITESPACE);
        append_branch_source(source, as_branch(term), term);
        append_phrase(source, term->stringPropOptional("syntax:whitespaceBeforeEnd", ""),
            term, token::WHITESPACE);
    }

    void evaluate_discard(EvalContext*, Term* caller)
    {
        Term* forTerm = caller->input(0);
        Term* discardCalled = get_for_loop_discard_called(forTerm);
        set_bool(discardCalled, true);
    }

    std::string discard_to_source_string(Term* term)
    {
        return "discard";
    }

    void discard_formatSource(RichSource* source, Term* term)
    {
        append_phrase(source, "discard", term, phrase_type::KEYWORD);
    }

    void setup(Branch& kernel)
    {
        FOR_FUNC = import_function(kernel, evaluate_for_loop, "for(List _state, List) -> Branch");
        function_t::get_attrs(FOR_FUNC).toSource = toSourceString;
        function_t::get_attrs(FOR_FUNC).formatSource = formatSource;
        function_t::set_input_meta(FOR_FUNC, 0, true); // allow _state to be NULL
        function_t::set_exposed_name_path(FOR_FUNC, "#rebinds_for_outer");

        DISCARD_FUNC = import_function(kernel, evaluate_discard, "discard(any)");
        function_t::get_attrs(DISCARD_FUNC).toSource = discard_to_source_string;
        function_t::get_attrs(DISCARD_FUNC).formatSource = discard_formatSource;
        hide_from_docs(DISCARD_FUNC);
    }
}
} // namespace circa
