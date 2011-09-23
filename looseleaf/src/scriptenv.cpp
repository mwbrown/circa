// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

#include "scriptenv.h"

using namespace circa;

circa::Branch g_globalEnv;

// Setup functions that are implemented elsewhere:
void window_setup(Branch* branch);

void initialize_script_env()
{
    circa_initialize();
    circa_use_default_filesystem_interface();

    // Load circa scripts
    parse_script(g_globalEnv, "src/window.ca");
    include_script(g_globalEnv, "../libs/opengl/opengl.ca");

    // Install C++ functions to scripts
    window_setup(&g_globalEnv);

    // Load 'main'
    parse_script(g_globalEnv, "runtime/main.ca");

    create_branch(g_globalEnv, "files");

    print_static_errors_formatted(g_globalEnv, std::cout);

    dump(g_globalEnv);
}

Branch* ScriptEnv::loadScript(const char* filename)
{
    branch = NULL;
    Branch* files = &nested_contents(g_globalEnv["files"]);
    if (files->get(filename) == NULL)
        branch = &create_branch(*files, filename);
    else
        branch = &nested_contents(files->get(filename));

    clear_branch(branch);
    parse_script(*branch, filename);

    print_static_errors_formatted(*branch, std::cout);

    return branch;
}

void ScriptEnv::tick()
{
    evaluate_branch(&context, *branch);
}

void destroy_script_env()
{
    clear_branch(&g_globalEnv);
    circa_shutdown();
}

