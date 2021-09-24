#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

/// NameManager is responsible for managing names to guarantee uniqueness.
///
// \ingroup Managers
class NameManager {
  public:
    /// Clears the manager of all names.
    void Reset();

    /// Adds a name, asserting that it is unique.
    void Add(const std::string &name);

    /// Removes a name, asserting that it was in there.
    void Remove(const std::string &name);

    /// Returns true if the given name is known.
    bool Find(const std::string &name) const;

    /// Creates and adds a new and unique name using the given prefix. For
    // example, passing a prefix of "Sphere" may create "Sphere_1" or another
    // name that is not already used.
    std::string Create(const std::string &prefix);

    /// Creates a name for a clone. The general strategy is to append an
    // underscore and capital letter (to distinguish from basic numbering
    // scheme).
    std::string CreateClone(const std::string &basename);

    /// Returns a sorted vector of all stored names.
    std::vector<std::string> GetAllNames() const;

  private:
    /// Stores unique names.
    std::unordered_set<std::string> name_set_;

    /// Given i >= 0, returns "A", "B", ..., "Z", "AA", "AB", etc.
    static std::string GetLetterSuffix_(int n);
};

typedef std::shared_ptr<NameManager> NameManagerPtr;
