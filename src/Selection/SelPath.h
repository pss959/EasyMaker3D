//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Math/Types.h"
#include "SG/CoordConv.h"
#include "SG/NodePath.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(Model);
DECL_SHARED_PTR(ParentModel);
DECL_SHARED_PTR(RootModel);

/// A SelPath is a derived SG::NodePath that represents selection of a
/// Model. It must begin at the RootModel and end at the selected Model.
///
/// \ingroup Selection
struct SelPath : public SG::NodePath {
    /// Default constructor.
    SelPath() {}

    /// Constructor that creates a path from the given RootModel to a top-level
    /// Model (child of the RootModel).
    SelPath(const RootModelPtr &root_model, const ModelPtr &child);

    /// Constructor that takes an SG::NodePath ending at a Model. This uses the
    /// subpath that starts at a RootModel. Asserts if the path does not work.
    SelPath(const SG::NodePath &node_path);

    /// Returns the selected Model (tail of the path). Asserts if the path is
    /// not valid.
    ModelPtr GetModel() const;

    /// Returns the parent of the selected Model (tail of the path), which may
    /// be the RootModel. Asserts if the path is not valid.
    ParentModelPtr GetParentModel() const;

    /// Validates the SelPath, asserting if anything is bad.
    void Validate() const;

    /// Returns a vector containing all Models in the path, from the root to
    /// the selected Model. If skip_root is true, the root is not returned.
    std::vector<ModelPtr> GetAllModels(bool skip_root) const;

    /// Returns true if this SelPath refers to an ancestor Model of SelPath p's
    /// Model.
    bool IsAncestorOf(const SelPath &p) const;

    /// Returns an SG::CoordConv instance that can be used to convert
    /// coordinates based on the path. Note that stage coordinates are
    /// equivalent to world coordinates for this SG::CoordConv.
    SG::CoordConv GetCoordConv() const;

    /// Returns a SelPath to a child of the selected Model. Asserts if the new
    /// SelPath would be ill-formed.
    SelPath GetPathToChild(const ModelPtr &child) const;
};
