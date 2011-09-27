// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

struct ForLoopContext
{
    bool discard;
    bool breakCalled;
    bool continueCalled;

    ForLoopContext() : discard(false), breakCalled(false), continueCalled(false) {}
};

Term* get_for_loop_iterator(Term* forTerm);
Term* get_for_loop_modify_list(Term* forTerm);
void setup_for_loop_pre_code(Term* forTerm);
Term* setup_for_loop_iterator(Term* forTerm, const char* name);
void setup_for_loop_post_code(Term* forTerm);

Term* find_enclosing_for_loop(Term* term);

void for_loop_update_output_index(Term* forTerm);

CA_FUNCTION(evaluate_for_loop);
void for_block_write_bytecode(Term* caller, BytecodeWriter* writer);
void for_block_write_bytecode_contents(Term* caller, BytecodeWriter* writer);

} // namespace circa
