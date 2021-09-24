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
    ModelPtr GetModel() {
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
};
