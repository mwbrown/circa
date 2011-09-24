// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"
#include "bytecode.h"
#include "types/dict.h"

namespace circa {
namespace get_state_field_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(get_state_field,
        "get_state_field(any container :optional, any default_value :optional) -> any")
    {
        Dict* stateContainer = NULL;

        // Check if a container was passed as input0
        if (INPUT_TERM(0) != NULL)
            stateContainer = Dict::checkCast(INPUT(0));

        if (stateContainer == NULL)
            stateContainer = get_scope_state(CONTEXT, 0);

        ca_assert(stateContainer != NULL);

        const char* name = CALLER->name.c_str();
        TaggedValue* value = stateContainer->get(name);

        // Try to cast 'value' to the declared type.
        if (value != NULL) {
            bool cast_success = cast(value, declared_type(CALLER), OUTPUT);
            if (cast_success)
                return;
        }

        // Try to use initialValue from an input.
        if (INPUT_TERM(1) != NULL) {
            // Evaluate nested contents first, since the initial value might come from there.
            // The initial value will either come as an input or from within our
            // nested contents.
            
            TaggedValue input;
            if (CALLER->nestedContents && nested_contents(CALLER).length() > 0) {
                Branch& contents = nested_contents(CALLER);
                push_stack_frame(CONTEXT, &contents);
                evaluate_branch_with_bytecode(CONTEXT, &contents);
                List* frame = get_stack_frame(CONTEXT, 0);

                // INPUT_INSTRUCTION_HACK
                InputInstruction* inputIsn = &CALLER->inputIsns.inputs[1];
                if (inputIsn->type == InputInstruction::GLOBAL)
                    copy(CALLER->input(1), &input);
                else
                    copy(frame->get(inputIsn->index), &input);

                pop_stack_frame(CONTEXT);
            } else {
                copy(INPUT(1), &input);
            }
            
            bool cast_success = cast(&input, declared_type(CALLER), OUTPUT);

            if (!cast_success) {
                std::stringstream msg;
                msg << "Couldn't cast default value from type "
                    << input.value_type->name << " to type " <<
                    declared_type(CALLER)->name;
                return error_occurred(CONTEXT, CALLER, msg.str());
            }

            return;
        }

        // Otherwise, reset to default value of type
        change_type(OUTPUT, declared_type(CALLER));
        reset(OUTPUT);
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

        if (defaultValue == NULL && nested_contents(term).length() > 0)
            defaultValue = nested_contents(term).getFromEnd(0);

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
        GET_STATE_FIELD_FUNC = kernel["get_state_field"];
        get_function_attrs(GET_STATE_FIELD_FUNC)->formatSource = formatSource;
    }
}
}
