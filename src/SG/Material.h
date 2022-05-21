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
    Parser::TField<Color> base_color_{"base_color",         Color::White()};
    Parser::TField<Color> emissive_color_{"emissive_color", Color::Clear()};
    Parser::TField<float> smoothness_{"smoothness", 0};
    Parser::TField<float> metalness_{"metalness", 0};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
