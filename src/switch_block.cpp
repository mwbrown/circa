// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
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

} // namespace circa
