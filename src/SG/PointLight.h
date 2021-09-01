#pragma once

#include "Math/Types.h"
#include "Parser/ObjectSpec.h"
#include "SG/Object.h"

namespace SG {

//! A PointLight object represents a point light source. It has no Ion
//! equivalent.
class PointLight : public Object {
  public:
    const Point3f  & GetPosition()  const { return position_;     }
    const Vector4f & GetColor()     const { return color_;        }
    bool             CastsShadows() const { return cast_shadows_; }

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed Fields
    //!@{
    Point3f  position_{0, 0, 0};
    Vector4f color_{ 1, 1, 1, 1 };
    bool     cast_shadows_ = true;
    //!@}
};

}  // namespace SG
