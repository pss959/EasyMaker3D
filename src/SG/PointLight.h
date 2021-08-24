#pragma once

#include "Parser/ObjectSpec.h"
#include "SG/Math.h"
#include "SG/Object.h"

namespace SG {

//! A PointLight object represents a point light source. It has no Ion
//! equivalent.
class PointLight : public Object {
  public:
    const Point3f  & GetPosition() const { return position_; }
    const Vector4f & GetColor()    const { return color_;    }

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed Fields
    //!@{
    Point3f  position_{0, 0, 0};
    Vector4f color_{ 1, 1, 1, 1 };
    //!@}
};

}  // namespace SG
