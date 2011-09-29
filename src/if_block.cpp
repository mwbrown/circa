// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <set>

#include "common_headers.h"

#include "branch.h"
#include "building.h"
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
}

int if_block_num_branches(Term* ifCall)
{
    return nested_contents(ifCall)->length() - 1;
}
Branch* if_block_get_branch(Term* ifCall, int index)
{
    return ifCall->contents(index)->contents();
}

void if_block_begin_branch(EvalContext* context)
{
    Term* caller = get_pc_term(context);
    Branch* contents = nested_contents(caller);

    // Walk across cases, find the one that succeeded
    int acceptedBranch = contents->length() - 2;
    for (int caseIndex=0; caseIndex < contents->length() - 2; caseIndex++) {
        Term* caseTerm = contents->get(caseIndex);
        TaggedValue* caseInput = get_input(context, caseTerm, 0);

        if (as_bool(caseInput)) {
            acceptedBranch = caseIndex;
            break;
        }
    }

    // Push stack frame
    Term* caseTerm = contents->get(acceptedBranch);
    Frame* frame = push_frame(context, nested_contents(caseTerm));
    frame->finishBranch = if_block_finish_branch;

    // Fetch local state
    Dict* outsideState = &get_frame(context, 1)->state;
    TaggedValue* stateEntry = outsideState->get(get_unique_name(caller));
    if (stateEntry != NULL) {
        List* stateList = List::lazyCast(stateEntry);
        if (acceptedBranch < stateList->length()) {
            TaggedValue* entry = stateList->get(acceptedBranch);
            if (!is_dict(entry))
                set_dict(entry);
            swap(entry, &frame->state);
        }

        const bool resetStateForUnusedBranches = true;
        if (resetStateForUnusedBranches)
            set_null(stateEntry);
    }
}

bool if_block_finish_branch(EvalContext* context, int flags)
{
    // Copy joined locals
    Term* caseTerm = top_frame(context)->branch->owningTerm;
    ca_assert(caseTerm->function == IF_FUNC || caseTerm->function == BRANCH_FUNC);

    int caseIndex = caseTerm->index;
    Term* caller = get_parent_term(caseTerm);
    Branch* contents = nested_contents(caller);
    Branch* joining = nested_contents(contents->getFromEnd(0));

    for (int i=0; i < joining->length(); i++) {
        Term* joinTerm = joining->get(i);
        Term* joinInput = joinTerm->input(i);

        TaggedValue* result = NULL;
        if (get_parent_term(joinInput) == caseTerm)
            result = get_output(context, joinInput);
        else
            result = get_input(context, joinTerm, caseIndex);
        TaggedValue* dest = get_extra_output_rel(context, caller, 1, i);
        copy(result, dest);
    }

    // Save local state
    Dict* prevScope = &get_frame(context, 1)->state;
    List* stateEntry = List::lazyCast(prevScope->insert(get_unique_name(caller)));

    if (stateEntry->length() <= caseIndex)
        stateEntry->resize(caseIndex + 1);

    swap(&get_frame(context, 0)->state, stateEntry->get(caseIndex));

    return true;
}

} // namespace circa
