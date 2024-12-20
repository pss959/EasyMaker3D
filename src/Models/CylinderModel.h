//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Models/PrimitiveModel.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(CylinderModel);

/// CylinderModel is a derived PrimitiveModel class representing a cylinder. It
/// has independent top and bottom radii. By default, both radii are 1 and Y
/// (the height dimension) ranges from -1 to 1.
///
/// A complexity 0 cylinder has 3 sides, while a complexity 1 cylinder has 120
/// sides.
///
/// \ingroup Models
class CylinderModel : public PrimitiveModel {
  public:
    /// Sets the top radius, clamped to be at least TK::kMinCylinderRadius.
    void SetTopRadius(float radius);

    /// Returns the current top radius value.
    float GetTopRadius() const { return top_radius_; }

    /// Sets the bottom radius, clamped to be at least TK::kMinCylinderRadius.
    void SetBottomRadius(float radius);

    /// Returns the current bottom radius value.
    float GetBottomRadius() const { return bottom_radius_; }

    /// CylinderModel responds to complexity.
    virtual bool CanSetComplexity() const override { return true; }

  protected:
    CylinderModel() {}
    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual TriMesh BuildMesh() override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<float> top_radius_;
    Parser::TField<float> bottom_radius_;
    ///@}

    friend class Parser::Registry;
};
