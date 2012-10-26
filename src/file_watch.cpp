// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "debug.h"
#include "file.h"
#include "list.h"
#include "names.h"
#include "native_modules.h"
#include "tagged_value.h"
#include "world.h"

namespace circa {

struct FileWatch
{
    Value filename;
    Value onChangeActions;
    int lastObservedMtime;
};

struct FileWatchWorld
{
    std::map<std::string, FileWatch*> watches;
};

FileWatchWorld* create_file_watch_world()
{
    FileWatchWorld* world = new FileWatchWorld();
    return world;
}

FileWatch* find_file_watch(World* world, const char* filename)
{
    std::map<std::string, FileWatch*>::const_iterator it =
        world->fileWatchWorld->watches.find(filename);

    if (it != world->fileWatchWorld->watches.end())
        return it->second;

    return NULL;
}

FileWatch* add_file_watch(World* world, const char* filename)
{
    // Return existing watch if it exists.
    FileWatch* existing = find_file_watch(world, filename);
    if (existing != NULL)
        return existing;

    FileWatch* newWatch = new FileWatch();
    set_string(&newWatch->filename, filename);
    set_list(&newWatch->onChangeActions, 0);
    newWatch->lastObservedMtime = 0;

    world->fileWatchWorld->watches[filename] = newWatch;
    return newWatch;
}

FileWatch* add_file_watch_action(World* world, const char* filename, Value* action)
{
    // Fetch the FileWatch entry.
    FileWatch* watch = add_file_watch(world, filename);

    // Check if this exact action already exists, if so do nothing.
    for (int i=0; i < list_length(&watch->onChangeActions); i++)
        if (equals(list_get(&watch->onChangeActions, i), action))
            return watch;

    // Add action
    copy(action, list_append(&watch->onChangeActions));

    return watch;
}

static bool file_watch_check_for_update(FileWatch* watch)
{
    int latestMtime = file_get_mtime(as_cstring(&watch->filename));
    if (latestMtime != watch->lastObservedMtime) {
        watch->lastObservedMtime = latestMtime;
        return true;
    }

    return false;
}

void file_watch_trigger_actions(World* world, FileWatch* watch)
{
    // Walk through each action and execute it.
    for (int i = 0; i < list_length(&watch->onChangeActions); i++) {
        caValue* action = list_get(&watch->onChangeActions, i);

        Name label = leading_name(action);
        ca_assert(label != name_None);

        switch (label) {
        case name_NativeModule: {
            NativeModule* nativeModule = add_native_module(world, as_cstring(&watch->filename));
            native_module_load_from_file(nativeModule, as_cstring(&watch->filename));

            caValue* moduleName = list_get(action, 1);
            Branch* branch = nested_contents(find_from_global_name(world, as_cstring(moduleName)));

            if (branch == NULL) {
                std::cout << "trying to apply native patch, couldn't find module: "
                    << as_cstring(moduleName) << std::endl;
                break;
            }

            native_module_apply_patch(nativeModule, branch);
            break;
        }
        case name_Branch: {
            // Reload this code branch.
            caValue* moduleName = list_get(action, 1);
            load_script_to_global_name(world, as_cstring(&watch->filename), as_cstring(moduleName));
            break;
        }
        default:
            internal_error("unrecognized file watch action");
        }
    }
}

void file_watch_trigger_actions(World* world, const char* filename)
{
    FileWatch* watch = find_file_watch(world, filename);

    // No-op if there is no watch.
    if (watch == NULL)
        return;

    file_watch_trigger_actions(world, watch);
}

void file_watch_check_now(World* world, FileWatch* watch)
{
    if (file_watch_check_for_update(watch))
        file_watch_trigger_actions(world, watch);
}

void file_watch_ignore_latest_change(FileWatch* watch)
{
    file_watch_check_for_update(watch);
}

void file_watch_check_all(World* world)
{
    std::map<std::string, FileWatch*>::const_iterator it;

    for (it = world->fileWatchWorld->watches.begin();
         it != world->fileWatchWorld->watches.end();
         ++it) {
        FileWatch* watch = it->second;
        file_watch_check_now(world, watch);
    }
}

FileWatch* add_file_watch_module_load(World* world, const char* filename, const char* moduleName)
{
    circa::Value action;
    set_list(&action, 2);
    set_name(list_get(&action, 0), name_Branch);
    set_string(list_get(&action, 1), moduleName);
    return add_file_watch_action(world, filename, &action);
}

FileWatch* add_file_watch_native_patch(World* world, const char* filename, const char* moduleName)
{
    circa::Value action;
    set_list(&action, 2);
    set_name(list_get(&action, 0), name_NativeModule);
    set_string(list_get(&action, 1), moduleName);
    return add_file_watch_action(world, filename, &action);
}

} // namespace circa
