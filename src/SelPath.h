#pragma once

#include "Assert.h"
#include "Models/Model.h"
#include "Models/RootModel.h"
#include "SG/NodePath.h"
#include "Util/General.h"

/// A SelPath is a derived SG::NodePath that represents selection of a
/// Model. It must begin at the RootModel and end at the selected Model.
struct SelPath : public SG::NodePath {
    /// Default constructor.
    SelPath() {}

    /// Returns the selected Model (tail of the path). Asserts if the path is
    /// not valid.
    ModelPtr GetModel() const {
        ASSERT(! empty());
        ModelPtr model = Util::CastToDerived<Model>(back());
        ASSERT(model);
        return model;
    }

    /// Validates the SelPath, asserting if anything is bad.
    void Validate() const {
        ASSERT(! empty());
        ASSERT(Util::IsA<RootModel>(front()));
        ASSERT(Util::CastToDerived<Model>(back()));
    }

    /// Returns a vector containing all Models in the path, from the root to
    /// the selected Model.
    std::vector<ModelPtr> GetAllModels() const {
        Validate();
        std::vector<ModelPtr> models;
        models.reserve(size());
        for (size_t i = 0; i < size(); ++i) {
            ModelPtr model = Util::CastToDerived<Model>((*this)[i]);
            ASSERT(model);
            models.push_back(model);
        }
        return models;
    }

    /// Returns true if this SelPath refers to an ancestor Model of SelPath p's
    /// Model.
    bool IsAncestorOf(const SelPath &p) const {
        Validate();
        p.Validate();
        // This is an ancestor if all nodes in the path are at the start of P.
        if (front() != p.front() || size() >= p.size())
            return false;
        for (size_t i = 0; i < size(); ++i)
            if ((*this)[i] != p[i])
                return false;
        return true;
    }
};
