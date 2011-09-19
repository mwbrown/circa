// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

#include "scriptenv.h"

using namespace circa;

ScriptEnv g_env;

// Setup functions that are implemented elsewhere:
void window_setup(Branch* branch);


void initialize_script_env()
{
    circa_initialize();

    // Load circa scripts
    parse_script(g_env.branch, "src/window.ca");

    // Install C++ functions to scripts
    window_setup(&g_env.branch);
}

void destroy_script_env()
{
    clear_branch(&g_env.branch);
    circa_shutdown();
}

