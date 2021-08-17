#pragma once

#include <ion/gfxutils/shapeutils.h>

#include "NParser/FieldSpec.h"
#include "SG/Math.h"
#include "SG/Shape.h"

namespace SG {

//! Rectangle is a derived Shape that represents a rectangle.
class Rectangle : public Shape {
  public:
    static std::vector<NParser::FieldSpec> GetFieldSpecs();

  private:
    typedef ion::gfxutils::PlanarShapeSpec::PlaneNormal PlaneNormal_;

    // Parsed fields.
    Vector2f     size_{ 2.f, 2.f };
    PlaneNormal_ plane_normal_ = PlaneNormal_::kPositiveZ;
};

}  // namespace SG
