// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_TERM_INCLUDED
#define CIRCA_TERM_INCLUDED

#include "common_headers.h"

#include "branch.h"
#include "references.h"
#include "ref_list.h"
#include "tagged_value.h"

namespace circa {

const int TERM_FLAG_ERRORED = 0x1;

struct List;

struct Term : TaggedValue
{
    // Inherited from TaggedValue:
    //   TaggedValueData value_data
    //   Type* value_type

    // A Type term that describes our data type
    Ref type;

    // Input terms
    RefList inputs;

    // Our function: the thing that takes our inputs and produces a value.
    Ref function;

    // Our name binding.
    std::string name;

    // The branch that owns this term. May be NULL
    Branch* owningBranch;

    // The index that this term currently holds inside owningBranch
    unsigned index;

    // A set of boolean flags
    unsigned flags;

    // A globally unique ID
    unsigned int globalID;

    // Dynamic properties
    Branch properties;

    // Reference count
    int refCount;

    // Terms which are using this term as an input.
    RefList users;

    Term();
    ~Term();

    Term* input(int index) const;
    int numInputs() const;

    std::string toString();

    // Returns the named property
    Term* property(std::string const& name) const;

    bool hasProperty(std::string const& name) const;
    Term* addProperty(std::string const& name, Term* type);
    void removeProperty(std::string const& name);

    int intProp(std::string const& name);
    float floatProp(std::string const& name);
    bool boolProp(std::string const& name);
    std::string const& stringProp(std::string const& name);
    Ref refProp(std::string const& name);

    void setIntProp(std::string const& name, int i);
    void setFloatProp(std::string const& name, float f);
    void setBoolProp(std::string const& name, bool b);
    void setStringProp(std::string const& name, std::string const& s);
    void setRefProp(std::string const& name, Term* r);

    int intPropOptional(std::string const& name, int defaultValue);
    float floatPropOptional(std::string const& name, float defaultValue);
    bool boolPropOptional(std::string const& name, bool defaultValue);
    std::string stringPropOptional(std::string const& name, std::string const& defaultValue);

    // Flag accessors
    bool hasError() const;
    void setHasError(bool error);

    // Convenience accessors
    int asInt();
    float asFloat();
    float toFloat();
    std::string const& asString();
    bool asBool();
    Ref& asRef();
    Branch& asBranch();

    Term* field(int index);
};

void assert_term_invariants(Term* t);

} // namespace circa

#endif
