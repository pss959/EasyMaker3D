#pragma once

#include <ion/gfxutils/shapeutils.h>

#include "Math/Types.h"
#include "SG/TriMeshShape.h"

namespace SG {

//! Polygon is a derived TriMeshShape that represents a regular polygon.
class Polygon : public TriMeshShape {
  public:
    typedef ion::gfxutils::PlanarShapeSpec::PlaneNormal PlaneNormal;

    virtual void AddFields() override;

    int         GetSides()       const { return sides_;        }
    PlaneNormal GetPlaneNormal() const { return plane_normal_; }

    virtual Bounds ComputeBounds() const override;
    virtual ion::gfx::ShapePtr CreateIonShape() override;

  private:
    //! \name Parsed fields.
    //!@{
    Parser::TField<int>            sides_{"sides"};
    Parser::EnumField<PlaneNormal> plane_normal_{
        "plane_normal", PlaneNormal::kPositiveZ};
    //!@}
};

}  // namespace SG
