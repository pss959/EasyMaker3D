//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Selection/SelPath.h"
#include "Util/Assert.h"
#include "Util/General.h"

/// The Selection class is used to store and pass around the set of currently
/// selected Models. Each selected Model is specified as a SelPath, which is a
/// NodePath from the RootModel to the selected Model.
///
/// \ingroup Selection
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
    size_t GetCount() const { return paths_.size(); }

    /// Returns the primary selection. Assert if there is none.
    const SelPath & GetPrimary() const {
        ASSERT(HasAny());
        return paths_.front();
    }

    /// Returns all selections, with the primary selection first.
    const std::vector<SelPath> & GetPaths() const { return paths_; }

    /// Returns a vector containing all selected Models, with the primary
    /// selection first.
    std::vector<ModelPtr> GetModels() const {
        return Util::ConvertVector<ModelPtr, SelPath>(
            paths_, [](const SelPath &p){ return p.GetModel(); });
    }

  private:
    /// Paths to all selected Models. If anything is selected, the primary
    /// selection is first in the list. This may be empty.
    std::vector<SelPath> paths_;
};
