#pragma once

#include <vector>

#include "SelPath.h"

/// The Selection class is used to store and pass around the set of currently
/// selected Models. Each selected Model is specified as a SelPath, which is a
/// NodePath from the RootModel to the selected Model.
class Selection {
  public:
    /// The default constructor creates an empty Selection.
    Selection() {}

    /// Constructor that creates a Selection with just the given Model selected.
    Selection(const SelPath &path) {
        path.Validate();
        paths_.push_back(path);
    }

    /// Clears the selection.
    void Clear() {
        paths_.clear();
    }

    /// Adds a Model to the selection.
    void Add(const SelPath &path) {
        path.Validate();
        paths_.push_back(path);
    }

    /// Returns true if anything is selected.
    bool HasAny() const { return ! paths_.empty(); }

    /// Returns the number of selected Models.
    size_t Count() const { return paths_.size(); }

    /// Returns the primary selection. Assert if there is none.
    const SelPath & GetPrimary() {
        ASSERT(HasAny());
        return paths_.front();
    }

    /// Returns all selections as a list, with the primary selection first.
    const std::vector<SelPath> GetAll() const { return paths_; }

  private:
    /// Paths to all selected Models. If anything is selected, the primary
    /// selection is first in the list. This may be empty.
    std::vector<SelPath> paths_;
};
