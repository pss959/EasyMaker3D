#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "SG/Object.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(Material);

/// The Material class is a shorthand for setting several uniforms used by the
/// Lighting shader.
///
/// \ingroup SG
class Material : public Object {
  public:
    const Color & GetBaseColor()     const { return base_color_; }
    const Color & GetEmissiveColor() const { return emissive_color_; }
    float         GetSmoothness()    const { return smoothness_; }
    float         GetMetalness()     const { return metalness_;  }

  protected:
    Material() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Color> base_color_;
    Parser::TField<Color> emissive_color_;
    Parser::TField<float> smoothness_;
    Parser::TField<float> metalness_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
