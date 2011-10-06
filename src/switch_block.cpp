// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "bytecode.h"
#include "if_block.h"
#include "importing_macros.h"
#include "kernel.h"
#include "list_shared.h"
#include "term.h"
#include "type.h"

namespace circa {

void switch_block_post_compile(Term* term)
{
    // Add a default case
    apply(nested_contents(term), DEFAULT_CASE_FUNC, TermList());
    update_if_block_joining_branch(term);
}

void switch_block_write_bytecode(Term* caller, BytecodeWriter* writer)
{
    Branch* contents = nested_contents(caller);
    Branch* parentBranch = caller->owningBranch;

    std::vector<int> jumpsToFinish;

    for (int caseIndex=0; caseIndex < contents->length()-1; caseIndex++) {
        Term* caseTerm = contents->get(caseIndex);

        int initial_jump = 0;
        if (caseTerm->function == CASE_FUNC) {
            initial_jump = bc_jump_if_not_equal(writer);
            bc_write_input(writer, parentBranch, caller->input(0));
            bc_write_input(writer, parentBranch, caseTerm->input(0));
        }

        bc_push_branch(writer, caseTerm);

        // Copy joined locals
        Branch* joining = nested_contents(contents->getFromEnd(0));

        for (int i=0; i < joining->length(); i++) {
            Term* joinTerm = joining->get(i);
            bc_copy(writer);
            bc_write_input(writer, nested_contents(caseTerm), joinTerm->input(caseIndex));
            bc_local_input(writer, 1, caller->index + 1 + i);
        }

        // Finish, clean up stack and wrap up jumps.
        bc_pop_branch(writer);

        jumpsToFinish.push_back(bc_jump(writer));

        if (caseTerm->function == CASE_FUNC)
            bc_jump_to_here(writer, initial_jump);
    }

    // Finish
    for (size_t i=0; i < jumpsToFinish.size(); i++)
        bc_jump_to_here(writer, jumpsToFinish[i]);
}

} // namespace circa
