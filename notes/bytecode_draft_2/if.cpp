
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
        bc_return_on_evaluation_interrupted(writer);

        jumpsToFinish.push_back(bc_jump(writer));

        if (caseTerm->function == IF_FUNC)
            bc_jump_to_here(writer, initial_jump);
    }

    // Finish
    for (size_t i=0; i < jumpsToFinish.size(); i++)
        bc_jump_to_here(writer, jumpsToFinish[i]);
}

