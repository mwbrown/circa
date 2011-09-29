
void for_block_write_bytecode(Term* caller, BytecodeWriter* writer)
{
    bc_call_branch(writer, caller);
    bc_pop_stack(writer);
    bc_return_on_evaluation_interrupted(writer);
}

void for_block_write_bytecode_contents(Term* caller, BytecodeWriter* writer)
{
    Branch* contents = nested_contents(caller);
    Branch* parentBranch = caller->owningBranch;
    bool useState = has_any_inlined_state(contents);
    Branch* outerRebinds = get_for_loop_outer_rebinds(caller);
    Term* inputList = caller->input(0);

    Term* indexTerm = contents->get(index_location);
    ca_assert(indexTerm->function == LOOP_INDEX_FUNC);

    // Prepare output value.
    // FIXME
    //bc_write_call_op(writer, caller, get_global("loop_prepare_output"));
    //bc_write_input(writer, contents, inputList);

    // Loop setup. Write 0 to the index.
    bc_call(writer, indexTerm);
    
    // Check if we are already finished (ie, iterating over an empty list)
    int jumpPastEmptyList = bc_jump_if_within_range(writer);
    bc_write_input(writer, contents, inputList);
    bc_write_input(writer, contents, indexTerm);

    // These instructions are evaluated when iterating over an empty list. Copy
    // joined locals appropriately.
    for (int i=0; i < outerRebinds->length(); i++) {
        bc_copy_value(writer);
        bc_write_input(writer, contents, outerRebinds->get(i)->input(0));
        bc_local_input(writer, 1, caller->index + 1 + i);
    }

    int jumpToEnd = bc_jump(writer);

    // Setup for first iteration. Copy inner rebinds from their outside sources.
    bc_jump_to_here(writer, jumpPastEmptyList);

    for (int i=inner_rebinds_location; contents->get(i)->function == JOIN_FUNC; i++) {
        Term* term = contents->get(i);
        bc_copy_value(writer);
        bc_write_input(writer, contents, term->input(0));
        bc_write_input(writer, contents, term);
    }

    int jumpPast2ndIterationSetup = bc_jump(writer);

    // For 2nd and later iterations, copy inner rebinds from the bottom of the loop.
    int secondIterationStart = writer->writePosition;

    for (int i=inner_rebinds_location; contents->get(i)->function == JOIN_FUNC; i++) {
        Term* term = contents->get(i);
        bc_copy_value(writer);
        bc_write_input(writer, contents, term->input(1));
        bc_write_input(writer, contents, term);
    }

    bc_jump_to_here(writer, jumpPast2ndIterationSetup);

    // Fetch iterator value.
    Term* iterator = contents->get(iterator_location);
    bc_write_call_op_with_func(writer, iterator, GET_INDEX_FUNC);
    bc_write_input(writer, contents, inputList);
    bc_write_input(writer, contents, indexTerm);

    // Loop contents
    for (int i = inner_rebinds_location; i < contents->length() - 1; i++)
        bc_call(writer, contents->get(i));

    // Save the list result
    Term* listResult = NULL;
    if (as_bool(get_for_loop_modify_list(caller)))
        listResult = contents->get(get_for_loop_iterator(caller)->name);
    else
        listResult = find_last_non_comment_expression(contents);
        
    bc_write_call_op_with_func(writer, caller, SET_INDEX_FUNC);
    bc_write_input(writer, contents, caller);
    bc_write_input(writer, contents, indexTerm);
    bc_write_input(writer, contents, listResult);

    // Finish iteration, increment index.
    bc_increment(writer);
    bc_local_input(writer, 0, indexTerm->index);

    // Jump back to the start of the loop (if we haven't reached the end).
    int jumpToStart = bc_jump_if_within_range(writer);
    bc_write_input(writer, contents, inputList);
    bc_write_input(writer, contents, indexTerm);
    bc_jump_to_pos(writer, jumpToStart, secondIterationStart);

    // Finished looping, copy outer rebinds
    for (int i=0; i < outerRebinds->length(); i++) {
        bc_copy_value(writer);
        bc_write_input(writer, contents, outerRebinds->get(i)->input(1));
        bc_local_input(writer, 1, caller->index + 1 + i);
    }

    // End
    bc_jump_to_here(writer, jumpToEnd);
}

