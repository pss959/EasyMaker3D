#pragma once

#include "Math/Types.h"
#include "Parser/ObjectSpec.h"
#include "SG/Object.h"

namespace SG {

//! The Material class is a shorthand for setting several uniforms used by the
//! Lighting shader.
class Material : public Object {
  public:
    const Vector4f & GetBaseColor()  const { return base_color_; }
    float            GetSmoothness() const { return smoothness_; }
    float            GetMetalness()  const { return metalness_;  }

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed Fields
    //!@{
    Vector4f base_color_;
    float    smoothness_ = 0;
    float    metalness_  = 0;
    //!@}
};

}  // namespace SG
