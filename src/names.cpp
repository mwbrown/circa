// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

/**
 names.cpp

 Open issues in this file:

  [1] the find functions should not call name_from_string, instead they should call
      existing_name_from_string. This currently fails for namespace-name searches,
      because the namespaced version of the name doesn't exist yet.
*/

#include "common_headers.h"

#include "branch.h"
#include "debug.h"
#include "kernel.h"
#include "heap_debugging.h"
#include "if_block.h"
#include "names_builtin.h"
#include "source_repro.h"
#include "term.h"

#include "names.h"

namespace circa {

struct RuntimeName
{
    char* str;
    Name namespaceFirst;
    Name namespaceRightRemainder;
};

const int c_FirstRuntimeName = name_LastBuiltinName + 1;
const int c_maxRuntimeNames = 2000;

RuntimeName g_runtimeNames[c_maxRuntimeNames];
int g_nextFreeNameIndex = 0;
std::map<std::string,Name> g_stringToSymbol;

bool exposes_nested_names(Term* term);

bool fits_lookup_type(Term* term, Name type)
{
    // Modules can only be found by LookupModule
    if (term->function == FUNCS.imported_file)
        return type == name_LookupModule;

    switch (type) {
        case name_LookupAny:
            return true;
        case name_LookupType:
            return is_type(term);
        case name_LookupFunction:
            return is_function(term);
        case name_LookupModule:
            return false;
    }
    internal_error("");
    return false;
}

Term* find_local_name(NameSearch* params)
{
    if (params->name == 0)
        return NULL;

    Branch* branch = params->branch;

    if (branch == NULL)
        return NULL;

    // position can be -1, meaning 'last term'
    int position = params->position;
    if (position == -1)
        position = branch->length();

    if (position > branch->length())
        position = branch->length();

    // Look for an exact match.
    for (int i = position - 1; i >= 0; i--) {
        Term* term = branch->get(i);
        if (term == NULL)
            continue;

        if (term->nameSymbol == params->name && fits_lookup_type(term, params->lookupType))
            return term;

        // If this term exposes its names, then search inside the nested branch.
        if (term->nestedContents != NULL && exposes_nested_names(term)) {
            NameSearch nestedSearch;
            nestedSearch.branch = term->nestedContents;
            nestedSearch.name = params->name;
            nestedSearch.position = -1;
            nestedSearch.lookupType = params->lookupType;

            Term* nested = find_local_name(&nestedSearch);
            if (nested != NULL)
                return nested;
        }
    }

    // Check if the name is a qualified name.
    Name namespacePrefix = qualified_name_get_first_section(params->name);

    // If it's not a qualified name, then quit.
    if (namespacePrefix == name_None)
        return NULL;

    // It is a qualified name, search for the first prefix.
    NameSearch nsSearch;
    nsSearch.branch = params->branch;
    nsSearch.name = namespacePrefix;
    nsSearch.position = params->position;
    nsSearch.lookupType = params->lookupType;
    Term* nsPrefixTerm = find_local_name(&nsSearch);

    // Give up if prefix not found
    if (nsPrefixTerm == NULL)
        return NULL;

    // Recursively search inside the prefix for the qualified suffix.
    NameSearch nestedSearch;
    nestedSearch.branch = nested_contents(nsPrefixTerm);
    nestedSearch.name = qualified_name_get_remainder_after_first_section(params->name);
    nestedSearch.position = -1;
    nestedSearch.lookupType = params->lookupType;
    return find_local_name(&nestedSearch);
}

Term* find_name(NameSearch* params)
{
    if (params->name == 0)
        return NULL;

    Branch* branch = params->branch;

    if (branch == NULL)
        branch = global_root_branch();

    INCREMENT_STAT(BranchNameLookups);

    Term* result = find_local_name(params);
    if (result != NULL)
        return result;

    // Name not found in this branch.

    // Don't continue the search if this is the kernel
    if (branch == global_root_branch())
        return NULL;

    // Search parent
    Term* parent = branch->owningTerm;
    if (parent != NULL) {
        // find_name with the parent's location plus one, so that we do look
        // at the parent's branch (in case our name has a namespace prefix
        // that refers to this branch).
        NameSearch parentSearch;
        parentSearch.branch = parent->owningBranch;
        parentSearch.name = params->name;
        parentSearch.position = parent->index + 1;
        parentSearch.lookupType = params->lookupType;
        return find_name(&parentSearch);
    }

    // No parent, search kernel
    return get_global(params->name);
}

Term* find_name(Branch* branch, Name name, int position, Name lookupType)
{
    NameSearch nameSearch;
    nameSearch.branch = branch;
    nameSearch.name = name;
    nameSearch.position = position;
    nameSearch.lookupType = lookupType;
    return find_name(&nameSearch);
}

// Finds a name in this branch.
Term* find_local_name(Branch* branch, Name name, int position, Name lookupType)
{
    NameSearch nameSearch;
    nameSearch.branch = branch;
    nameSearch.name = name;
    nameSearch.position = position;
    nameSearch.lookupType = lookupType;
    return find_local_name(&nameSearch);
}

Term* find_name(Branch* branch, const char* nameStr, int position, Name lookupType)
{
    Name name = name_from_string(nameStr);
    if (name == name_None)
        return NULL;

    NameSearch nameSearch;
    nameSearch.branch = branch;
    nameSearch.name = name;
    nameSearch.position = position;
    nameSearch.lookupType = lookupType;
    return find_name(&nameSearch);
}

Term* find_local_name(Branch* branch, const char* nameStr, int position, Name lookupType)
{
    Name name = name_from_string(nameStr);
    if (name == name_None)
        return NULL;

    NameSearch nameSearch;
    nameSearch.branch = branch;
    nameSearch.name = name;
    nameSearch.position = position;
    nameSearch.lookupType = lookupType;
    return find_local_name(&nameSearch);
}

Term* find_name_at(Term* term, const char* nameStr)
{
    Name name = name_from_string(nameStr);
    if (name == name_None)
        return NULL;

    NameSearch nameSearch;
    nameSearch.branch = term->owningBranch;
    nameSearch.name = name;
    nameSearch.position = term->index;
    nameSearch.lookupType = name_LookupAny;
    return find_name(&nameSearch);
}

Term* find_name_at(Term* term, Name name)
{
    NameSearch nameSearch;
    nameSearch.branch = term->owningBranch;
    nameSearch.name = name;
    nameSearch.position = term->index;
    nameSearch.lookupType = name_LookupAny;
    return find_name(&nameSearch);
}

int find_qualified_name_separator(const char* name)
{
    for (int i=0; name[i] != 0; i++) {
        if (name[i] == ':' && name[i+1] != 0)
            return i;
    }
    return -1;
}

bool exposes_nested_names(Term* term)
{
    if (term->nestedContents == NULL)
        return false;
    if (nested_contents(term)->length() == 0)
        return false;
    if (term->function == FUNCS.include_func)
        return true;
    if (term->function == FUNCS.imported_file)
        return true;

    return false;
}

Term* get_global(Name name)
{
    return find_name(global_root_branch(), name);
}

Term* get_global(const char* nameStr)
{
    Name name = name_from_string(nameStr);
    if (name == name_None)
        return NULL;

    return get_global(name);
}

Branch* get_parent_branch(Branch* branch)
{
    if (branch == global_root_branch())
        return NULL;

    if (branch->owningTerm == NULL)
        return global_root_branch();

    if (branch->owningTerm->owningBranch == NULL)
        return global_root_branch();

    return branch->owningTerm->owningBranch;
}

Term* get_parent_term(Term* term)
{
    if (term->owningBranch == NULL)
        return NULL;
    if (term->owningBranch->owningTerm == NULL)
        return NULL;

    return term->owningBranch->owningTerm;
}

Term* get_parent_term(Term* term, int levels)
{
    for (int i=0; i < levels; i++) {
        term = get_parent_term(term);
        if (term == NULL)
            return NULL;
    }
    return term;
}

bool name_is_reachable_from(Term* term, Branch* branch)
{
    if (term->owningBranch == branch)
        return true;

    Branch* parent = get_parent_branch(branch);

    if (parent == NULL)
        return false;

    return name_is_reachable_from(term, parent);
}

Branch* find_first_common_branch(Term* left, Term* right)
{
    Branch* leftParent = left->owningBranch;
    Branch* rightParent = right->owningBranch;

    if (leftParent == NULL) return NULL;
    if (rightParent == NULL) return NULL;

    // Walk upwards from left term.
    while (leftParent != NULL && leftParent != global_root_branch()) {

        // Walk upwards from right term.
        while (rightParent != NULL && leftParent != global_root_branch()) {
            if (leftParent == rightParent)
                return leftParent;

            rightParent = get_parent_branch(rightParent);
        }

        leftParent = get_parent_branch(leftParent);
        rightParent = right->owningBranch;
    }

    return NULL;
}

bool term_is_child_of_branch(Term* term, Branch* branch)
{
    while (term != NULL) {
        if (term->owningBranch == branch)
            return true;

        term = get_parent_term(term);
    }

    return false;
}

// Returns whether or not we succeeded
bool get_relative_name_recursive(Branch* branch, Term* term, std::stringstream& output)
{
    if (name_is_reachable_from(term, branch)) {
        output << term->name;
        return true;
    }

    Term* parentTerm = get_parent_term(term);

    if (parentTerm == NULL)
        return false;

    // Don't include the names of hidden branches
    if (is_hidden(parentTerm)) {
        output << term->name;
        return true;
    }

    bool success = get_relative_name_recursive(branch, parentTerm, output);

    if (success) {
        output << ":" << term->name;
        return true;
    } else {
        return false;
    }
}

std::string get_relative_name(Branch* branch, Term* term)
{
    ca_assert(term != NULL);

    if (name_is_reachable_from(term, branch))
        return term->name;

    // Build a dot-separated name
    std::stringstream result;

    get_relative_name_recursive(branch, term, result);

    return result.str();
}

std::string get_relative_name_at(Term* location, Term* term)
{
    if (location == NULL)
        return get_relative_name(global_root_branch(), term);

    if (location->owningBranch == NULL)
        return term->name;
    else
        return get_relative_name(location->owningBranch, term);
}

void update_unique_name(Term* term)
{
    Term::UniqueName& name = term->uniqueName;

    if (term->owningBranch == NULL) {
        name.name = term->name;
        return;
    }

    name.base = term->name;

    if (name.base == "") {
        if (term->function == NULL)
            name.base = "_anon";
        else
            name.base = "_" + term->function->name;
    }

    name.name = name.base;
    name.ordinal = 0;

    // Look for a name collision. We might need to keep looping, if our generated name
    // collides with an existing name.

    Branch* branch = term->owningBranch;

    bool updatedName = true;
    while (updatedName) {
        updatedName = false;

        for (int i = term->index-1; i >= 0; i--) {
            Term* other = branch->get(i);
            if (other == NULL) continue;

            // If another term shares the same base, then make sure our ordinal is
            // higher. This turns some O(n) cases into O(1)
            if ((other->uniqueName.base == name.base)
                    && (other->uniqueName.ordinal >= name.ordinal)) {
                name.ordinal = other->uniqueName.ordinal + 1;
                updatedName = true;

            // If this name is already used, then just try the next ordinal. This
            // case results in more blind searching, but it's necessary to handle
            // the situation where a generated name is already taken.
            } else if (other->uniqueName.name == name.name) {
                name.ordinal++;
                updatedName = true;
            }

            if (updatedName) {
                char ordinalBuf[30];
                sprintf(ordinalBuf, "%d", name.ordinal);
                name.name = name.base + "_" + ordinalBuf;
                break;
            }
        }
    }
}

const char* get_unique_name(Term* term)
{
    return term->uniqueName.name.c_str();
}

Term* find_from_unique_name(Branch* branch, const char* name)
{
    // O(n) search; this should be made more efficient.

    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (term == NULL)
            continue;
        if (strcmp(get_unique_name(term), name) == 0) {
            return branch->get(i);
        }
    }
    return NULL;
}

bool find_global_name(Term* term, std::string& name)
{
    // Search upwards, check if this term even has a global name.
    Term* searchTerm = term;

    std::vector<Term*> stack;

    while (true) {
        stack.push_back(searchTerm);

        if (searchTerm->owningBranch == global_root_branch())
            break;

        searchTerm = get_parent_term(searchTerm);

        if (searchTerm == NULL)
            return false;
    }

    // Construct a qualified name.
    std::stringstream out;

    for (int i = stack.size()-1; i >= 0; i--) {
        out << stack[i]->uniqueName.name;
        if (i > 0)
            out << ":";
    }
    name = out.str();
    return true;
}
std::string find_global_name(Term* term)
{
    std::string out;
    find_global_name(term, out);
    return out;
}

Term* find_term_from_global_name_recr(Branch* searchBranch, const char* name)
{
    int separator = find_qualified_name_separator(name);
    
    if (separator == -1)
        return find_from_unique_name(searchBranch, name);

    std::string namePortion = std::string(name, separator);
    
    Term* searchTerm = find_from_unique_name(searchBranch, namePortion.c_str());
    if (searchTerm == NULL)
        return NULL;
    if (searchTerm->nestedContents == NULL)
        return NULL;

    return find_term_from_global_name_recr(searchTerm->nestedContents,
            &name[separator+1]);
}

Term* find_term_from_global_name(const char* name)
{
    Branch* searchBranch = global_root_branch();
    return find_term_from_global_name_recr(searchBranch, name);
}

bool name_is_valid(Name name)
{
    if (name < 0)
        return false;

    const char* builtin = builtin_name_to_string(name);
    if (builtin != NULL)
        return true;

    int runtimeIndex = name - c_FirstRuntimeName;
    if (runtimeIndex < 0 || runtimeIndex >= g_nextFreeNameIndex)
        return false;

    return true;
}

const char* name_to_string(Name name)
{
    const char* builtin = builtin_name_to_string(name);
    if (builtin != NULL)
        return builtin;

    // Runtime symbols
    if (name >= c_FirstRuntimeName)
        return g_runtimeNames[name - c_FirstRuntimeName].str;

    internal_error("Unknown name in name_to_string");
    return "";
}

void name_to_string(Name name, String* string)
{
    set_string((caValue*) string, name_to_string(name));
}

Name qualified_name_get_first_section(Name name)
{
    if (name < c_FirstRuntimeName)
        return 0;
    else
        return g_runtimeNames[name - c_FirstRuntimeName].namespaceFirst;
}

Name qualified_name_get_remainder_after_first_section(Name name)
{
    if (name < c_FirstRuntimeName)
        return 0;
    else
        return g_runtimeNames[name - c_FirstRuntimeName].namespaceRightRemainder;
}

Name as_name(caValue* tv)
{
    return tv->value_data.asint;
}

void set_name(caValue* tv, Name val)
{
    ca_assert(name_is_valid(val));
    set_null(tv);
    tv->value_type = &NAME_T;
    tv->value_data.asint = val;
}

Name existing_name_from_string(const char* str)
{
    INCREMENT_STAT(InternedNameLookup);

    std::map<std::string,Name>::const_iterator it;
    it = g_stringToSymbol.find(str);
    if (it != g_stringToSymbol.end())
        return it->second;

    return 0;
}

// Runtime symbols
Name name_from_string(const char* str)
{
    // Empty string is name_None
    if (str[0] == 0)
        return name_None;

    // Check if name is already registered
    Name existing = existing_name_from_string(str);
    if (existing != name_None)
        return existing;

    // Not yet registered; add it to the list.
    INCREMENT_STAT(InternedNameCreate);

    Name index = g_nextFreeNameIndex++;
    g_runtimeNames[index].str = strdup(str);
    g_runtimeNames[index].namespaceFirst = 0;
    g_runtimeNames[index].namespaceRightRemainder = 0;
    Name name = index + c_FirstRuntimeName;
    g_stringToSymbol[str] = name;

    // Search the string for a : name, if found we'll update the name's
    // namespace links.
    int len = strlen(str);
    for (int i=0; i < len; i++) {
        if (str[i] == ':') {
            // Create a temporary string to hold the substring before :
            char* tempstr = (char*) malloc(i + 1);
            memcpy(tempstr, str, i);
            tempstr[i] = 0;

            g_runtimeNames[index].namespaceFirst = name_from_string(tempstr);
            g_runtimeNames[index].namespaceRightRemainder = name_from_string(str + i + 1);
            free(tempstr);
            break;
        }
    }

    return name;
}
Name name_from_string(std::string const& str)
{
    return name_from_string(str.c_str());
}
Name name_from_string(caValue* str)
{
    return name_from_string(as_cstring(str));
}
void name_dealloc_global_data()
{
    for (int i=0; i < g_nextFreeNameIndex; i++)
        free(g_runtimeNames[i].str);
    g_nextFreeNameIndex = 0;
    g_stringToSymbol.clear();
}

} // namespace circa

extern "C" caName circa_to_name(const char* str)
{
    return circa::name_from_string(str);
}
extern "C" const char* circa_name_to_string(caName name)
{
    return circa::name_to_string(name);
}
