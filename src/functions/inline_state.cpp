// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"
#include "types/dict.h"

namespace circa {
namespace inline_state_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(get_state_field,
        "get_state_field(any container +optional, any default_value +optional) -> any")
    {
        Dict* stateContainer = NULL;

        TaggedValue *containerFromInput = INPUT(0);
        if (containerFromInput != NULL)
            stateContainer = Dict::checkCast(containerFromInput);

        if (stateContainer == NULL)
            stateContainer = Dict::lazyCast(&CONTEXT->currentScopeState);

        ca_assert(stateContainer != NULL);

        const char* name = CALLER->name.c_str();
        TaggedValue* value = stateContainer->get(name);

        if (value) {
            // TODO: check if we need to cast this value
            copy(value, OUTPUT);

        // If we didn't find the value, see if they provided a default
        } else if (INPUT(1) != NULL) {
            copy(INPUT(1), OUTPUT);

        // Otherwise, reset to default value of type
        } else {
            ca_assert(CALLER != NULL);
            change_type(OUTPUT, unbox_type(CALLER->type));
            reset(OUTPUT);
        }

        // append name to the list of open state vars
        set_string(CONTEXT->openStateVariables.append(), name);
    }

    CA_DEFINE_FUNCTION(set_state_field,
            "set_state_field(any container, string name, any field) -> any")
    {
        copy(INPUT(0), OUTPUT);
        TaggedValue *container = OUTPUT;
        touch(container);
        if (!is_dict(container)) make_dict(container);
        Dict* dict = Dict::checkCast(container);
        dict->set(STRING_INPUT(1), INPUT(2));
    }

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "state ", term, token::STATE);

        if (term->hasProperty("syntax:explicitType")) {
            append_phrase(source, term->stringProp("syntax:explicitType"),
                    term, phrase_type::TYPE_NAME);
            append_phrase(source, " ", term, token::WHITESPACE);
        }

        append_phrase(source, term->name.c_str(), term, phrase_type::TERM_NAME);

        Term* defaultValue = term->input(1);
        if (defaultValue != NULL) {
            append_phrase(source, " = ", term, phrase_type::UNDEFINED);
            if (defaultValue->name != "")
                append_phrase(source, get_relative_name(term, defaultValue),
                        term, phrase_type::TERM_NAME);
            else
                format_term_source(source, defaultValue);
        }
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        function_t::get_attrs(kernel["get_state_field"]).formatSource =
            formatSource;
    }
}
}
