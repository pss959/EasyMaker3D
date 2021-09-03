#pragma once

#include "Math/Types.h"
#include "SG/Object.h"

namespace SG {

//! The Material class is a shorthand for setting several uniforms used by the
//! Lighting shader.
class Material : public Object {
  public:
    virtual void AddFields() override;

    const Vector4f & GetBaseColor()  const { return base_color_; }
    float            GetSmoothness() const { return smoothness_; }
    float            GetMetalness()  const { return metalness_;  }

  private:
    //! \name Parsed Fields
    //!@{
    Parser::TField<Vector4f> base_color_{"base_color", {1, 1, 1, 1}};
    Parser::TField<float>    smoothness_{"smoothness", 0};
    Parser::TField<float>    metalness_{"metalness", 0};
    //!@}
};

}  // namespace SG
