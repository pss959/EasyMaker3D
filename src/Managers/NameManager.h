#pragma once

#include <unordered_set>
#include <vector>

#include "Agents/NameAgent.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(NameManager);

/// NameManager is responsible for managing names to guarantee uniqueness.
///
/// \ingroup Managers
class NameManager : public NameAgent {
  public:
    /// Clears the manager of all names.
    void Reset();

    /// Adds a name, asserting that it is unique.
    virtual void Add(const Str &name) override;

    /// Removes a name, asserting that it was in there.
    void Remove(const Str &name);

    /// Returns true if the given name is known.
    virtual bool Find(const Str &name) const override;

    /// Creates and adds a new and unique name using the given prefix. For
    // example, passing a prefix of "Sphere" may create "Sphere_1" or another
    // name that is not already used.
    Str Create(const Str &prefix);

    /// Creates a name for a clone. The general strategy is to append an
    // underscore and capital letter (to distinguish from basic numbering
    // scheme).
    Str CreateClone(const Str &basename);

    /// Returns a sorted vector of all stored names.
    StrVec GetAllNames() const;

  private:
    /// Stores unique names.
    std::unordered_set<Str> name_set_;

    /// Given i >= 0, returns "A", "B", ..., "Z", "AA", "AB", etc.
    static Str GetLetterSuffix_(int n);
};
