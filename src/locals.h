// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {
    
int get_output_count(Term* term);
int get_locals_count(Branch& branch);
void update_locals_index_for_new_term(Term*);
void refresh_locals_indices(Branch&, int startingAt = 0);
void update_output_count(Term* term);

int get_frame_distance(Branch* frame, Term* input);
int get_frame_distance(Term* term, Term* input);

void update_input_instructions(Term* term);
void update_input_instructions(Branch& branch);

} // namespace circa
