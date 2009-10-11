// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace include_function {

    void load_script(Term* caller)
    {
        Term* state = caller->input(0);
        std::string &prev_filename = state->asBranch()[0]->asString();
        int &prev_modified = state->asBranch()[1]->asInt();

        std::string &requested_filename = caller->input(1)->asString();

        time_t modifiedTime = get_modified_time(requested_filename);

        // Reload if the filename or modified-time has changed
        if (requested_filename != prev_filename
                || prev_modified != modifiedTime) {
            prev_filename = requested_filename;
            prev_modified = (int) modifiedTime;

            Branch& contents = as_branch(caller);
            Branch previous_contents;
            duplicate_branch(contents, previous_contents);

            contents.clear();

            std::string filename = get_path_relative_to_source(caller, requested_filename);

            if (!file_exists(filename)) {
                error_occurred(caller, "File not found: "+filename);
                return;
            }

            parse_script(contents, filename);

            if (previous_contents.length() > 0)
                migrate_stateful_values(previous_contents, contents);

            if (has_static_errors(contents)) {
                error_occurred(caller, get_static_errors_formatted(contents));
                return;
            }
        }
    }

    void evaluate_include(Term* caller)
    {
        load_script(caller);

        if (caller->hasError())
            return;

        Branch& contents = as_branch(caller);
        evaluate_branch(contents, caller);
    }

    std::string toSourceString(Term* term)
    {
        return "include " + get_source_of_input(term, 1);
    }

    void setup(Branch& kernel)
    {
        parse_type(kernel, "type _file_signature { string filename, int time_modified }");
        INCLUDE_FUNC = import_function(kernel, evaluate_include,
                "include(state _file_signature, string filename) : Branch");
        function_t::get_to_source_string(INCLUDE_FUNC) = toSourceString;

        import_function(kernel, load_script,
            "load_script(state _file_signature, string filename) : Branch");
    }
}
} // namespace circa
