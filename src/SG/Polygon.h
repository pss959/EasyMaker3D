#pragma once

#include <ion/gfxutils/shapeutils.h>

#include "NParser/FieldSpec.h"
#include "SG/Math.h"
#include "SG/Shape.h"

namespace SG {

//! Polygon is a derived Shape that represents a regular polygon.
class Polygon : public Shape {
  public:
    static std::vector<NParser::FieldSpec> GetFieldSpecs();

  private:
    typedef ion::gfxutils::PlanarShapeSpec::PlaneNormal PlaneNormal_;

    //! \name Parsed fields.
    //!@{
    int          sides_;
    PlaneNormal_ plane_normal_ = PlaneNormal_::kPositiveZ;
    //!@}
};

}  // namespace SG
