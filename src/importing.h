// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// importing.cpp : Functions to help import C functions into Circa.

#pragma once

#include "common_headers.h"

#include "function.h"
#include "token_stream.h"

namespace circa {

// Create a Circa function, using the given C evaluation function, and
// a header in Circa-syntax.
//
// Example function header: "function do-something(int, string) -> int"
Term* import_function(Branch& branch, EvaluateFunc func, std::string const& header);

// Install an evaluate function into an existing function object.
void install_function(Term* function, EvaluateFunc evaluate);

Term* import_type(Branch& branch, Type* type);

Term* find_term_from_mangled_name(Term* term, const char* name);
void install_function(Branch* branch, EvaluateFunc evaluate, const char* name);

} // namespace circa
