// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <set>

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "bytecode.h"
#include "evaluation.h"
#include "importing_macros.h"
#include "kernel.h"
#include "list_shared.h"
#include "locals.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"
#include "update_cascades.h"

#include "if_block.h"

namespace circa {

// The format of if_block is as follows:
//
// N = branch length
//
// {
//   [0] if(cond0) : Branch
//   [1] if(cond1) : Branch
//   ...
//   [N-2] branch()  (this corresponds to the 'else' block)
//   [N-1] #joining = branch() 
//

void update_if_block_joining_branch(Term* ifCall)
{
    Branch* contents = nested_contents(ifCall);

    // Create the joining contents if necessary
    if (!contents->contains("#joining"))
        create_branch(contents, "#joining");

    Branch* joining = nested_contents(contents->get("#joining"));
    clear_branch(joining);

    // Find the set of all names bound in every branch.
    std::set<std::string> boundNames;

    for (int branch_index=0; branch_index < contents->length()-1; branch_index++) {
        Term* term = contents->get(branch_index);
        Branch* branch = nested_contents(term);

        TermNamespace::const_iterator it;
        for (it = branch->names.begin(); it != branch->names.end(); ++it) {
            std::string const& name = it->first;

            // Ignore empty or hidden names
            if (name == "" || name[0] == '#') {
                continue;
            }

            boundNames.insert(it->first);
        }
    }

    Branch* outerScope = ifCall->owningBranch;
    ca_assert(outerScope != NULL);

    // Filter out some names from boundNames.
    for (std::set<std::string>::iterator it = boundNames.begin(); it != boundNames.end();)
    {
        std::string const& name = *it;

        // We only rebind names that are either 1) already bound in the outer scope, or
        // 2) bound in every possible branch.
        
        bool boundInOuterScope = find_name(outerScope, name.c_str()) != NULL;

        bool boundInEveryBranch = true;

        for (int branch_index=0; branch_index < contents->length()-1; branch_index++) {
            Branch* branch = nested_contents(contents->get(branch_index));
            if (!branch->contains(name))
                boundInEveryBranch = false;
        }

        if (!boundInOuterScope && !boundInEveryBranch)
            boundNames.erase(it++);
        else
            ++it;
    }

    int numBranches = contents->length() - 1;

    // For each name, create a term that selects the correct version of this name.
    for (std::set<std::string>::const_iterator it = boundNames.begin();
            it != boundNames.end();
            ++it)
    {
        std::string const& name = *it;

        TermList inputs;
        inputs.resize(numBranches);

        Term* outerVersion = get_named_at(ifCall, name);

        for (int i=0; i < numBranches; i++) {
            Term* local = contents->get(i)->contents(name.c_str());
            inputs.setAt(i, local == NULL ? outerVersion : local);
        }

        apply(joining, JOIN_FUNC, inputs, name);
    }

    finish_update_cascade(joining);
    update_input_instructions(ifCall);
}

int if_block_num_branches(Term* ifCall)
{
    return nested_contents(ifCall)->length() - 1;
}
Branch* if_block_get_branch(Term* ifCall, int index)
{
    return ifCall->contents(index)->contents();
}

#if 0
void if_block_write_bytecode(Term* caller, BytecodeWriter* writer)
{
    Branch* contents = nested_contents(caller);
    Branch* parentBranch = caller->owningBranch;
    bool useState = has_any_inlined_state(contents);

    // Keep track of OP_JUMPs that jump to the end.
    std::vector<int> jumpsToFinish;

    // Write each case
    for (int caseIndex=0; caseIndex < contents->length()-1; caseIndex++) {
        Term* caseTerm = contents->get(caseIndex);

        int initial_jump = 0;
        if (caseTerm->function == IF_FUNC) {
            initial_jump = bc_jump_if_not(writer);
            bc_write_input(writer, parentBranch, caseTerm->input(0));
        }

        if (useState) {
            bc_write_call_op_with_func(writer, caller, IF_BLOCK_UNPACK_STATE_FUNC);
            bc_write_int_input(writer, caseIndex);
        }

        bc_call_branch(writer, caseTerm);

        if (useState) {
            bc_write_call_op_with_func(writer, caller, IF_BLOCK_PACK_STATE_FUNC);
            bc_write_int_input(writer, caseIndex);
        }

        // Copy joined locals
        Branch* joining = nested_contents(contents->getFromEnd(0));

        for (int i=0; i < joining->length(); i++) {
            Term* joinTerm = joining->get(i);
            bc_copy_value(writer);
            bc_write_input(writer, nested_contents(caseTerm), joinTerm->input(caseIndex));
            bc_local_input(writer, 1, caller->index + 1 + i);
        }

        // Finish, clean up stack and wrap up jumps.
        bc_pop_stack(writer);

        jumpsToFinish.push_back(bc_jump(writer));

        if (caseTerm->function == IF_FUNC)
            bc_jump_to_here(writer, initial_jump);
    }

    // Finish
    for (size_t i=0; i < jumpsToFinish.size(); i++)
        bc_jump_to_here(writer, jumpsToFinish[i]);
}
#endif

void if_block_write_calling_bytecode(Term* term, BytecodeWriter* writer)
{
}

void if_block_write_nested_bytecode(BytecodeWriter* writer, Term* term)
{
}

} // namespace circa
