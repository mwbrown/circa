// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

Term* for_loop_get_iterator(Term* forTerm);
void setup_for_loop_pre_code(Term* forTerm);
void for_loop_rename_iterator(Term* forTerm, const char* name);
void setup_for_loop_post_code(Term* forTerm);

Term* find_enclosing_for_loop(Term* term);

void for_block_write_bytecode(Term* caller, BytecodeWriter* writer);
void for_block_write_bytecode_contents(Term* caller, BytecodeWriter* writer);

} // namespace circa
