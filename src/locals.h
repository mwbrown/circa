// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {
    
int get_extra_output_count(Term* term);

int get_frame_distance(Branch* frame, Term* input);
int get_frame_distance(Term* term, Term* input);

//void update_input_instructions(Term* term);
//void update_input_instructions(Branch* branch);

} // namespace circa
