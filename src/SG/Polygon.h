#pragma once

#include <ion/gfxutils/shapeutils.h>

#include "NParser/ObjectSpec.h"
#include "SG/Math.h"
#include "SG/Shape.h"

namespace SG {

//! Polygon is a derived Shape that represents a regular polygon.
class Polygon : public Shape {
  public:
    typedef ion::gfxutils::PlanarShapeSpec::PlaneNormal PlaneNormal;

    int         GetSides()       const { return sides_;        }
    PlaneNormal GetPlaneNormal() const { return plane_normal_; }

    virtual ion::gfx::ShapePtr CreateIonShape() override;

    static NParser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed fields.
    //!@{
    int         sides_;
    PlaneNormal plane_normal_ = PlaneNormal::kPositiveZ;
    //!@}
};

}  // namespace SG
