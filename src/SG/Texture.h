#pragma once

#include <ion/gfx/texture.h>

#include "NParser/ObjectSpec.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"

namespace SG {

//! The Texture class wraps an Ion Texture.
class Texture : public Object {
  public:
    //! Returns the associated Ion texture.
    const ion::gfx::TexturePtr & GetIonTexture() { return ion_texture_; }

    // XXXX
    const std::string & GetUniformName() const { return uniform_name_; }
    const ImagePtr    & GetImage()       const { return image_; }
    const SamplerPtr  & GetSampler()     const { return sampler_; }

    static NParser::ObjectSpec GetObjectSpec();

  private:
    ion::gfx::TexturePtr ion_texture_;

    // Parsed fields.
    std::string uniform_name_;
    ImagePtr    image_;
    SamplerPtr  sampler_;

    //! Redefines this to set up the Ion Texture.
    virtual void Finalize() override;
};

}  // namespace SG
