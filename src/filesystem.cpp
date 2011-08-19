// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "filesystem.h"
#include "term.h"

namespace circa {

StorageInterface g_storageInterface;

void read_text_file_to_value(const char* filename, TaggedValue* contents, TaggedValue* error)
{
    struct ReceiveFile {
        TaggedValue* _contents;
        TaggedValue* _error;
        static void Func(void* context, const char* contents, const char* error) {
            ReceiveFile* obj = (ReceiveFile*) context;

            if (contents == NULL)
                set_string(obj->_contents, "");
            else
                set_string(obj->_contents, contents);

            if (obj->_error != NULL) {
                if (error == NULL)
                    set_null(obj->_error);
                else
                    set_string(obj->_error, error);
            }
        }
    };

    ReceiveFile obj;
    obj._contents = contents;
    obj._error = error;

    read_text_file(filename, ReceiveFile::Func, &obj);
}

std::string read_text_file_as_str(const char* filename)
{
    TaggedValue contents;
    read_text_file_to_value(filename, &contents, NULL);

    if (is_string(&contents))
        return as_string(&contents);
    return "";
}

void install_storage_interface(StorageInterface* interface)
{
    g_storageInterface = *interface;
}

void get_current_storage_interface(StorageInterface* interface)
{
    *interface = g_storageInterface;
}

std::string get_directory_for_filename(std::string const& filename)
{
    // TODO: This function is terrible, need to use an existing library for dealing
    // with paths.
    size_t last_slash = filename.find_last_of("/");

    if (last_slash == filename.npos)
        return ".";

    if (last_slash == 0)
        return "/";

    std::string result = filename.substr(0, last_slash);

    return result;
}

bool is_absolute_path(std::string const& path)
{
    // TODO: This function is terrible, need to use an existing library for dealing
    // with paths.
    
    if (path.length() >= 1 && path[0] == '/')
        return true;
    if (path.length() >= 2 && path[1] == ':')
        return true;
    return false;
}

std::string get_absolute_path(std::string const& path)
{
    if (is_absolute_path(path))
        return path;

    char buf[512];
#ifdef WINDOWS
    std::string cwd = _getcwd(buf, 512);
#else
    std::string cwd = getcwd(buf, 512);
#endif

    return cwd + "/" + path;
}

void read_text_file(const char* filename, FileReceiveFunc receiveFile, void* context)
{
    if (g_storageInterface.readTextFile == NULL)
        return;
    return g_storageInterface.readTextFile(filename, receiveFile, context);
}

void write_text_file(const char* filename, const char* contents)
{
    if (g_storageInterface.writeTextFile == NULL)
        return;
    return g_storageInterface.writeTextFile(filename, contents);
}

time_t get_modified_time(const char* filename)
{
    if (filename[0] == 0)
        return 0;

    if (g_storageInterface.getModifiedTime == NULL)
        return 0;

    return g_storageInterface.getModifiedTime(filename);
}

bool file_exists(const char* filename)
{
    if (g_storageInterface.fileExists == NULL)
        return false;
    return g_storageInterface.fileExists(filename);
}

} // namespace circa

// defined in filesystem_posix.cpp:
void install_posix_filesystem_interface();

void circa_use_default_filesystem_interface()
{
    install_posix_filesystem_interface();
}