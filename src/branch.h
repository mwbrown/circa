// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include "names.h"
#include "tagged_value.h"
#include "term_list.h"
#include "term_namespace.h"
#include "types/list.h"

namespace circa {

struct BrokenLinkList;

struct Branch
{
    // List of content terms. This branch owns all the Term objects in this list.
    TermList _terms;

    TermNamespace names;

    // Points to the Term which owns this branch as a value.
    Term* owningTerm;

    int _refCount;

    // Deprecated:
    int outputIndex;

    // Number of local values.
    int localsCount;

    // Variant value describing where this branch came from. 
    //   If the branch came from a file, then the value will be of format:
    //     [:file, String filename, int nullable_timestamp]
    TaggedValue origin;

    // Whether this branch has inlined state inside its contents or any nested contents.
    // This value will either be a boolean, or a null (indicating that the value needs to
    // be recalculated).
    TaggedValue hasInlinedState;

    // If this branch has any static errors, then they are listed here. If there
    // are no errors then this value is null.
    // If this has a list, each element has structure:
    //  [0] int index
    //  [1] string type
    //  [2] int inputIndex (only used for errors related to inputs)
    TaggedValue staticErrors;

    // This value stores information about pending code changes. This is populated
    // when someone makes a term change that will force other terms to be updated.
    // See update_cascades.cpp
    TaggedValue pendingUpdates;
    bool currentlyCascadingUpdates;

    // Bytecode data. This might be NULL if bytecode has never been created.
    BytecodeData* bytecode;

    Branch();
    ~Branch();

    int length();

    Term* get(int index);

    // Get an index counting from the end; equivalent to get(length() - index - 1)
    Term* getFromEnd(int index);

    Term* operator[](int index) { return get(index); }

    // Get a term from a name binding.
    inline Term* get(std::string const& name) { return find_local_name(this, name.c_str()); }
    inline Term* getNamed(const char* name) { return find_local_name(this, name); }
    inline Term* operator[](std::string const& name) { return find_local_name(this, name.c_str()); }

    // Returns true if there is a term with the given name
    bool contains(std::string const& name);

    int getIndex(Term* term);
    int debugFindIndex(Term* term);

    Term* last();

    // Find the first term with the given name binding.
    Term* findFirstBinding(std::string const& name);

    // Find the last term with the given name binding.
    Term* findLastBinding(std::string const& name);

    // Find a term with the given name, returns -1 if not found.
    int findIndex(std::string const& name);
    int findIndex(const char* name);

    void insert(int index, Term* term);
    void append(Term* term);
    Term* appendNew();
    void insertEmptySlots(int index, int count);

    void set(int index, Term* term);
    void setNull(int index);

    void move(Term* term, int index);
    void moveToEnd(Term* term);

    void remove(int index);
    void remove(std::string const& name);
    void removeNulls();
    void removeNameBinding(Term* term);
    void shorten(int newLength);
    void clear();

    // Bind a name to a term
    void bindName(Term* term, std::string name);

    // Remap pointers
    void remapPointers(TermMap const& map);

    // Compile the given statement, return the result term.
    Term* compile(std::string const& statement);

    // Evaluate the given statement, return the result value.
    Term* eval(std::string const& statement);

    std::string toString();

private:
    // Disabled calls
    Branch(Branch const&) {}
    Branch& operator=(Branch const&) { return *this; }
};

void assert_valid_branch(Branch const* obj);

bool is_namespace(Term* term);
bool is_namespace(Branch* branch);

bool has_nested_contents(Term* term);
Branch* nested_contents(Term* term);

std::string get_branch_source_filename(Branch* branch);
Branch* get_outer_scope(Branch* branch);

// Delete this term and remove it from its owning branch.
void erase_term(Term* term);

// Delete the contents of 'branch'.
void clear_branch(Branch* branch);

void duplicate_branch(Branch* source, Branch* dest);

void load_script(Branch* branch, const char* filename);
void evaluate_script(Branch* branch, const char* filename);

// Create an include() call that loads the given file. Returns the included
// branch.
Branch* include_script(Branch* branch, const char* filename);

// Create a load_script() call that loads the given file. The load_script
// function doesn't evaluate the contents when called. Returns the included
// branch.
Branch* load_script_term(Branch* branch, const char* filename);

// Save the script's text back to its file origin.
void save_script(Branch* branch);

Term* find_term_by_id(Branch* branch, unsigned int id);

void persist_branch_to_file(Branch* branch);
std::string get_source_file_location(Branch* branch);

// Returns a List pointer if the branch has a file origin, NULL if not.
List* branch_get_file_origin(Branch* branch);

// Checks Branch.origin, and checks the modified time of 'filename'. If the origin
// does not match the file's modified time, then we return true and update the
// branch's origin. So, if this function true then the branch should be reloaded.
bool check_and_update_file_origin(Branch* branch, const char* filename);

// If this branch has a file origin, then check the backing file, and if it's
// newer, update the branch with the latest contents. Returns true if the
// branch contents were updated.
bool refresh_script(Branch* branch);

struct BranchInvariantCheck
{
    List errors;

    // Structure of each error:
    // [0] int type
    // [1] int index
    // [2] string message
};

void branch_check_invariants(BranchInvariantCheck* result, Branch* branch);
bool branch_check_invariants_print_result(Branch* branch, std::ostream& out);

struct BrokenLinkList
{
    struct Link {
        std::string relativeName;
        Term* user;
        int depIndex;
        Link(std::string _relativeName, Term* _user, int _depIndex)
            : relativeName(_relativeName), user(_user), depIndex(_depIndex)
        {}
    };

    std::vector<Link> links;

    void append(std::string relativeName, Term* user, int depIndex)
    {
        links.push_back(Link(relativeName, user, depIndex));
    }
    bool empty() { return links.size() == 0; }
};

} // namespace circa
