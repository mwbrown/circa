// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

void finish_if_block_minor_branch(Term* branch);
void update_if_block_joining_branch(Term* ifCall);

int if_block_num_branches(Term* ifCall);
Branch* if_block_get_branch(Term* ifCall, int index);

CA_FUNCTION(evaluate_if_block);

void if_block_write_bytecode(Term* caller, BytecodeWriter* writer);

} // namespace circa
