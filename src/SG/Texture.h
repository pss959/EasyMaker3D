#pragma once

#include <ion/gfx/texture.h>

#include "SG/Image.h"
#include "SG/Object.h"
#include "SG/Sampler.h"
#include "SG/Typedefs.h"

namespace SG {

/// The Texture class wraps an Ion Texture.
class Texture : public Object {
  public:
    virtual void AddFields() override;

    /// Returns the Ion Texture for this instance. This will be null
    /// until CreateIonTexture() is called.
    const ion::gfx::TexturePtr & GetIonTexture() const { return ion_texture_; }

    /// Creates and stores an empty Ion Texture.
    void CreateIonTexture();

    int                 GetCount()       const { return count_; }
    const std::string & GetUniformName() const { return uniform_name_; }
    const ImagePtr    & GetImage()       const { return image_; }
    const SamplerPtr  & GetSampler()     const { return sampler_; }

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<int>          count_{"count", 1};
    Parser::TField<std::string>  uniform_name_{"uniform_name"};
    Parser::ObjectField<Image>   image_{"image"};
    Parser::ObjectField<Sampler> sampler_{"sampler"};
    ///@}

    ion::gfx::TexturePtr ion_texture_;
};

}  // namespace SG
