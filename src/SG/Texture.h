#pragma once

#include <ion/gfx/texture.h>

#include "SG/Image.h"
#include "SG/Object.h"
#include "SG/Sampler.h"
#include "SG/Typedefs.h"

namespace SG {

//! The Texture class wraps an Ion Texture.
class Texture : public Object {
  public:
    virtual void AddFields() override;

    //! Returns the associated Ion texture.
    const ion::gfx::TexturePtr & GetIonTexture() const { return ion_texture_; }

    int                 GetCount()       const { return count_; }
    const std::string & GetUniformName() const { return uniform_name_; }
    const ImagePtr    & GetImage()       const { return image_; }
    const SamplerPtr  & GetSampler()     const { return sampler_; }

    virtual void SetUpIon(IonContext &context) override;

  private:
    ion::gfx::TexturePtr ion_texture_;

    //! \name Parsed Fields
    //!@{
    Parser::TField<int>          count_{"count", 1};
    Parser::TField<std::string>  uniform_name_{"uniform_name"};
    Parser::ObjectField<Image>   image_{"image"};
    Parser::ObjectField<Sampler> sampler_{"sampler"};
    //!@}
};

}  // namespace SG
