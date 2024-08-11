//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <ion/gfxutils/shapeutils.h>

#include "Math/Types.h"
#include "SG/TriMeshShape.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(RegularPolygon);

/// RegularPolygon is a derived TriMeshShape that represents a regular
/// polygon. It is an equilateral triangle in the Z=0 plane by default. This is
/// derived from TriMeshShape to make intersection testing easier.
///
/// \ingroup SG
class RegularPolygon : public TriMeshShape {
  public:
    using PlaneNormal = ion::gfxutils::PlanarShapeSpec::PlaneNormal;

    int         GetSides()       const { return sides_;        }
    PlaneNormal GetPlaneNormal() const { return plane_normal_; }

  protected:
    RegularPolygon() {}
    virtual void AddFields() override;
    virtual Bounds ComputeBounds() const override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<int>            sides_;
    Parser::EnumField<PlaneNormal> plane_normal_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
