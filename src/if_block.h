// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

void finish_if_block_minor_branch(Term* branch);
void update_if_block_joining_branch(Term* ifCall);

int if_block_num_branches(Term* ifCall);
Branch* if_block_get_branch(Term* ifCall, int index);

void if_block_begin_branch(EvalContext* context);
bool if_block_finish_branch(EvalContext* context, int flags);

} // namespace circa
