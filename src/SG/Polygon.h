#pragma once

#include <ion/gfxutils/shapeutils.h>

#include "Parser/ObjectSpec.h"
#include "SG/Math.h"
#include "SG/TriMeshShape.h"

namespace SG {

//! Polygon is a derived TriMeshShape that represents a regular polygon.
class Polygon : public TriMeshShape {
  public:
    typedef ion::gfxutils::PlanarShapeSpec::PlaneNormal PlaneNormal;

    int         GetSides()       const { return sides_;        }
    PlaneNormal GetPlaneNormal() const { return plane_normal_; }

    virtual Bounds ComputeBounds() const override;
    virtual ion::gfx::ShapePtr CreateIonShape() override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed fields.
    //!@{
    int         sides_;
    PlaneNormal plane_normal_ = PlaneNormal::kPositiveZ;
    //!@}
};

}  // namespace SG
