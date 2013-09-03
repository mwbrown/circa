// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "building.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "function.h"
#include "generic.h"
#include "kernel.h"
#include "inspection.h"
#include "interpreter.h"
#include "list.h"
#include "native_patch.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "string_type.h"
#include "names.h"
#include "term.h"
#include "term_list.h"
#include "token.h"
#include "type.h"
#include "update_cascades.h"
#include "world.h"

namespace circa {

Term* create_function(Block* block, const char* name)
{
    ca_assert(name != NULL);
    Term* term = apply(block, FUNCS.function_decl, TermList(), name);
    return term;
}

void finish_building_function(Block* contents)
{
    // Connect the primary output placeholder with the last expression.
    Term* primaryOutput = get_output_placeholder(contents, 0);
    ca_assert(primaryOutput->input(0) == NULL);
    Term* lastExpression = find_last_non_comment_expression(contents);
    set_input(primaryOutput, 0, lastExpression);

    // Make output type more specific.
    if (primaryOutput->type == TYPES.any && lastExpression != NULL)
        change_declared_type(primaryOutput, lastExpression->type);

    // Write a list of output_placeholder terms.

    // Look at every input declared as :output, these will be used to declare extra outputs.
    // TODO is a way to declare extra outputs that are not rebound inputs.
    for (int i = count_input_placeholders(contents) - 1; i >= 0; i--) {
        Term* input = get_input_placeholder(contents, i);

        if (input->boolProp("output", false)) {

            Term* result = find_name(contents, input->name.c_str());
            
            Term* output = append_output_placeholder(contents, result);
            rename(output, &input->nameValue);
            change_declared_type(output, input->type);
            output->setIntProp("rebindsInput", i);
        }
    }

    // After the output_placeholder terms are created, we might need to update any
    // recursive calls.

    for (BlockIterator it(contents); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if (function_contents(term->function) != contents)
            continue;

        // Update extra outputs
        update_extra_outputs(term);
    }

    update_for_control_flow(contents);

    // Possibly apply a native patch
    module_possibly_patch_new_function(global_world(), contents);

    block_finish_changes(contents);
}

Type* derive_specialized_output_type(Term* function, Term* call)
{
    if (!is_function(function))
        return TYPES.any;

    Block* contents = function_contents(function);
    Type* outputType = get_output_type(contents, 0);

    if (contents->overrides.specializeType != NULL)
        outputType = contents->overrides.specializeType(call);
    if (outputType == NULL)
        outputType = TYPES.any;

    if (function->boolProp("preferSpecialize", false)) {
        Term* specialized = statically_specialize_overload_for_call(call);
        if (specialized != NULL)
            return get_output_type(function_contents(specialized), 0);
    }
    return outputType;
}

bool function_call_rebinds_input(Term* term, int index)
{
    return get_input_syntax_hint_optional(term, index, "rebindInput", "") == "t";
}

void function_format_header_source(caValue* source, Block* function)
{
    Term* term = function->owningTerm;

    ca_assert(term != NULL);

    append_phrase(source, term->name, term, sym_TermName);

    append_phrase(source, term->stringProp("syntax:postNameWs", ""),
            term, tok_Whitespace);
    append_phrase(source, term->stringProp("syntax:properties", ""),
            term, sym_None);

    append_phrase(source, "(", term, tok_LParen);

    bool first = true;
    int numInputs = count_input_placeholders(function);
    for (int i=0; i < numInputs; i++) {

        Term* input = get_input_placeholder(function, i);

        std::string name = input->name;

        if (input->boolProp("hiddenInput", false))
            continue;

        if (input->boolProp("state", false))
            append_phrase(source, "state ", term, sym_None);

        if (!first)
            append_phrase(source, ", ", term, sym_None);
        first = false;

        // Type (may be omitted)
        if (input->boolProp("syntax:explicitType", true)) {
            append_phrase(source, as_cstring(&input->type->name),
                input->type->declaringTerm, sym_TypeName);
            append_phrase(source, " ", term, tok_Whitespace);
        }

        // Name
        if (input->boolProp("syntax:rebindSymbol", false))
            append_phrase(source, "@", term, sym_None);

        append_phrase(source, name, term, sym_None);

        if (input->boolProp("output", false)
                && !input->boolProp("syntax:rebindSymbol", false)) {
            append_phrase(source, " ", term, tok_Whitespace);
            append_phrase(source, ":out", term, sym_None);
        }

        if (input->boolProp("meta", false)) {
            append_phrase(source, " ", term, tok_Whitespace);
            append_phrase(source, ":meta", term, sym_None);
        }

        if (input->boolProp("rebind", false)) {
            append_phrase(source, " ", term, tok_Whitespace);
            append_phrase(source, ":rebind", term, sym_None);
        }

        if (input->boolProp("multiple", false)) {
            append_phrase(source, " ", term, tok_Whitespace);
            append_phrase(source, ":multiple", term, sym_None);
        }
    }

    append_phrase(source, ")", term, tok_LParen);

    if (term->boolProp("syntax:explicitType", false)) {
        append_phrase(source, term->stringProp("syntax:whitespacePreColon", ""),
                term, tok_Whitespace);
        append_phrase(source, "->", term, sym_None);
        append_phrase(source, term->stringProp("syntax:whitespacePostColon", ""),
                term, tok_Whitespace);

        int unhiddenOutputCount = 0;

        for (int i=0;; i++) {
            Term* output = get_output_placeholder(function, i);
            if (output == NULL)
                break;
            if (is_hidden(output))
                continue;
            unhiddenOutputCount++;
        }

        bool multiOutputSyntax = unhiddenOutputCount > 1;

        if (multiOutputSyntax)
            append_phrase(source, "(", term, sym_None);

        for (int i=0;; i++) {

            Term* output = get_output_placeholder(function, i);
            if (output == NULL)
                break;
            if (is_hidden(output))
                continue;

            if (i > 0)
                append_phrase(source, ", ", term, sym_None);

            append_phrase(source, as_cstring(&output->type->name),
                output->type->declaringTerm, sym_TypeName);
        }

        if (multiOutputSyntax)
            append_phrase(source, ")", term, sym_None);
    }
}

void function_format_source(caValue* source, Term* term)
{
    append_phrase(source, "def ", term, tok_Def);

    Block* contents = function_contents(term);

    function_format_header_source(source, contents);
    format_block_source(source, contents, term);
}

void evaluate_subroutine(caStack*)
{
    // This once did something, but now the default function calling behavior
    // is the same as evaluating a subroutine.
}

bool is_subroutine(Term* term)
{
    if (!is_function(term))
        return false;
    return function_contents(term)->overrides.evaluate == NULL;
}

bool is_subroutine(Block* block)
{
    return block->owningTerm != NULL && is_subroutine(block->owningTerm);
}

} // namespace circa
