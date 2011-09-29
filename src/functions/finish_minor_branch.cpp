// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "circa.h"

#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace finish_minor_branch_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(finish_minor_branch_func, "finish_minor_branch()")
    {
#if 0 // FIXME
        Branch* contents = nested_contents(CALLER);
        push_stack_frame(CONTEXT, contents);
        evaluate_branch_with_bytecode(CONTEXT, contents);
        pop_stack_frame(CONTEXT);
#endif
    }

    void postCompile(Term* finishBranchTerm)
    {
        Branch* contents = nested_contents(finishBranchTerm);
        clear_branch(contents);

        Branch* outerContents = finishBranchTerm->owningBranch;

        // Find every state var that was opened in this branch, and add a
        // preserve_state_result() call for each.
        for (int i=0; i < outerContents->length(); i++) {
            Term* term = outerContents->get(i);

            if (term == NULL)
                continue;

            if (term->function == GET_STATE_FIELD_FUNC) {
                if (term->name == "")
                    continue;
                Term* outcome = get_named_at(finishBranchTerm, term->name);
                apply(contents, PRESERVE_STATE_RESULT_FUNC, TermList(outcome));
            }
        }
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        FINISH_MINOR_BRANCH_FUNC = kernel->get("finish_minor_branch");
        get_function_attrs(FINISH_MINOR_BRANCH_FUNC)->postCompile = postCompile;
    }
}
}
