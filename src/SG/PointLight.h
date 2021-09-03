#pragma once

#include "Math/Types.h"
#include "SG/Object.h"

namespace SG {

//! A PointLight object represents a point light source. It has no Ion
//! equivalent.
class PointLight : public Object {
  public:
    virtual void AddFields() override;

    const Point3f  & GetPosition()  const { return position_;     }
    const Vector4f & GetColor()     const { return color_;        }
    bool             CastsShadows() const { return cast_shadows_; }

  private:
    //! \name Parsed Fields
    //!@{
    Parser::TField<Point3f>  position_{"position", {0, 0, 0}};
    Parser::TField<Vector4f> color_{"color", {1, 1, 1, 1}};
    Parser::TField<bool>     cast_shadows_{"cast_shadows", true};
    //!@}
};

}  // namespace SG
