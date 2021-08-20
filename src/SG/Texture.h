#pragma once

#include <ion/gfx/texture.h>

#include "Parser/ObjectSpec.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"

namespace SG {

//! The Texture class wraps an Ion Texture.
class Texture : public Object {
  public:
    //! Returns the associated Ion texture.
    const ion::gfx::TexturePtr & GetIonTexture() { return ion_texture_; }

    const std::string & GetUniformName() const { return uniform_name_; }
    const ImagePtr    & GetImage()       const { return image_; }
    const SamplerPtr  & GetSampler()     const { return sampler_; }

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    ion::gfx::TexturePtr ion_texture_;

    //! \name Parsed Fields
    //!@{
    std::string uniform_name_;
    ImagePtr    image_;
    SamplerPtr  sampler_;
    //!@}
};

}  // namespace SG
