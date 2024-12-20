//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "SG/TriMeshShape.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(Torus);

/// Torus is a derived TriMeshShape that represents a torus aligned with the Y
/// axis, defined by inner and outer radii. The outer radius includes the
/// circular inner cross section; an outer radius of 10 produces a torus that
/// is 20 units in X and Z.
///
/// \ingroup SG
class Torus : public TriMeshShape {
  public:
    float GetOuterRadius() const { return outer_radius_; }
    float GetInnerRadius() const { return inner_radius_; }
    int   GetRingCount()   const { return ring_count_;   }
    int   GetSectorCount() const { return sector_count_; }

    /// Sets the inner radius of the torus.
    void SetInnerRadius(float radius);

    /// Sets the outer radius of the torus.
    void SetOuterRadius(float radius);

    /// Sets all configurable values at the same time so that the mesh is
    /// rebuilt only once.
    void SetGeometry(float inner_radius, float outer_radius,
                     size_t ring_count, size_t sector_count);

  protected:
    Torus() {}
    virtual void AddFields() override;
    virtual Bounds ComputeBounds() const override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<float> outer_radius_;
    Parser::TField<float> inner_radius_;
    Parser::TField<int>   ring_count_;
    Parser::TField<int>   sector_count_;
    ///@}

    /// Updates the Ion Shape (if it exists) when fields change.
    void UpdateIonShape_();

    /// Builds a TriMesh using the current field values.
    TriMesh BuildMesh_() const;

    friend class Parser::Registry;
};

}  // namespace SG
