#pragma once

#include "Math/Types.h"
#include "SG/Object.h"

namespace Parser { class Registry; }

namespace SG {

/// A PointLight object represents a point light source. It has no Ion
/// equivalent.
class PointLight : public Object {
  public:
    virtual void AddFields() override;

    const Point3f  & GetPosition()  const { return position_;     }
    const Color    & GetColor()     const { return color_;        }
    bool             CastsShadows() const { return cast_shadows_; }

  protected:
    PointLight() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Point3f>  position_{"position", {0, 0, 0}};
    Parser::TField<Color>    color_{"color", Color::White()};
    Parser::TField<bool>     cast_shadows_{"cast_shadows", true};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
