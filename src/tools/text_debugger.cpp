// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "evaluation.h"
#include "introspection.h"
#include "source_repro.h"
#include "static_checking.h"
#include "term.h"

namespace circa {

void print_debugger_state(EvalContext* context, int frameIndex, int indent)
{
    std::ostream &out = std::cout;

    Frame* frame = get_frame(context, frameIndex);
    Branch* branch = frame->branch;

    for (int index=0; index < branch->length(); index++) {
        if (frame->pc == index)
            out << "> ";
        else
            out << "  ";

        for (int i=0; i < indent; i++)
            out << " ";

        Term* term = branch->get(index);

        out << index << ": " << global_id(term);
        if (term->name != "")
           out << " '" << term->name << "'";
       
        out << " " << term->function->name;
        // Arguments
        out << "(";
        for (int i=0; i < term->numInputs(); i++) {
            if (i != 0) out << " ";
            out << global_id(term->input(i));
        }
        out << ")";

        //out << " | ";
        //out << get_term_source_text(term);
        out << " | ";
        if (is_value(term))
            out << term->toString();
        else
            out << frame->locals.get(index)->toString();

        out << std::endl;

        // If the PC is here then recursively print the next frame here
        if (frame->pc == index && frameIndex > 0)
            print_debugger_state(context, frameIndex - 1, indent + 1);
    }
}

int run_text_debugger(const char* filename)
{
    Branch branch;
    load_script(&branch, filename);

    if (print_static_errors_formatted(&branch, std::cout))
        return -1;

    EvalContext context;
    interpreter_start(&context, &branch);

    while (!interpreter_finished(&context)) {

        print_debugger_state(&context, context.numFrames-1, 0);

        std::cout << "> ";

        std::string input;
        if (!std::getline(std::cin, input))
            break;

        interpreter_step(&context);
    }
    return 0;
}

} // namespace circa
