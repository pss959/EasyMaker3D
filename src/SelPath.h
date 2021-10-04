#pragma once

#include "Models/Model.h"
#include "Models/RootModel.h"
#include "SG/NodePath.h"

/// A SelPath is a derived SG::NodePath that represents selection of a
/// Model. It must begin at the RootModel and end at the selected Model.
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

    /// Validates the SelPath, asserting if anything is bad.
    void Validate() const;

    /// Returns a vector containing all Models in the path, from the root to
    /// the selected Model.
    std::vector<ModelPtr> GetAllModels() const;

    /// Returns true if this SelPath refers to an ancestor Model of SelPath p's
    /// Model.
    bool IsAncestorOf(const SelPath &p) const;
};
