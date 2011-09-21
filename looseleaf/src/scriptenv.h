// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "circa.h"

struct ScriptEnv
{
    circa::Branch* branch;
    circa::EvalContext context;

    void loadScript(const char* filename);
    void tick();
};

void initialize_script_env();
void run_script();
void destroy_script_env();
