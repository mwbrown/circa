// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "evaluation.h"
#include "introspection.h"
#include "parser.h"
#include "static_checking.h"
#include "term.h"

namespace circa {

void repl_evaluate_line(Branch& branch, std::string const& input, std::ostream& output)
{
    int previousHead = branch.length();
    parser::compile(branch, parser::statement_list, input);
    int newHead = branch.length();

    bool anyErrors = false;

    int resultIndex = -1;

    // Look for static errors
    for (int i=previousHead; i < newHead; i++) {
        Term* result = branch[i];

        if (has_static_error(result)) {
            output << "error: ";
            print_static_error(result, output);
            output << std::endl;
            anyErrors = true;
            break;
        }
    }

    EvalContext context;
    context.preserveLocals = true;
    if (!anyErrors)
        evaluate_range(&context, branch, previousHead, newHead);

    // Print results of the last expression
    if (!anyErrors && resultIndex != -1) {
        Term* result = branch[resultIndex];
        if (result->type != as_type(VOID_TYPE)) {
            output << ((TaggedValue*) result)->toString() << std::endl;
        }
    }
}

int run_repl()
{
    Branch replState;
    bool displayRaw = false;

    while (true) {
        std::cout << "> ";

        std::string input;

        if (!std::getline(std::cin, input))
            break;

        if (input == "exit" || input == "/exit")
            break;

        if (input == "")
            continue;

        if (input == "/raw") {
            displayRaw = !displayRaw;
            if (displayRaw) std::cout << "Displaying raw output" << std::endl;
            else std::cout << "Not displaying raw output" << std::endl;
            continue;
        }
        if (input == "/clear") {
            clear_branch(&replState);
            std::cout << "Cleared working area" << std::endl;
            continue;
        }

        if (input == "/help") {
            std::cout << "Special commands: /raw, /help, /exit" << std::endl;
            continue;
        }

        int previousHead = replState.length();
        repl_evaluate_line(replState, input, std::cout);

        if (displayRaw) {
            for (int i=previousHead; i < replState.length(); i++) {
                std::cout << get_term_to_string_extended(replState[i]) << std::endl;
                if (nested_contents(replState[i]).length() > 0)
                    print_branch(std::cout, nested_contents(replState[i]));
            }
        }
    }

    return 0;
}

} // namespace circa
