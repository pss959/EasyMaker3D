#pragma once

#include <ion/gfx/texture.h>

#include "Graph/Object.h"
#include "Graph/Typedefs.h"

namespace Input { class Extractor; }

namespace Graph {

//! The Texture class wraps an Ion Texture.
//!
//! \ingroup Graph
class Texture : public Object {
  public:
    Texture();

    //! Returns the associated Ion texture.
    const ion::gfx::TexturePtr &GetIonTexture() { return i_texture_; }

    // XXXX
    const std::string & GetUniformName() const { return uniform_name_; }
    const ImagePtr    & GetImage()       const { return image_; }

  private:
    ion::gfx::TexturePtr i_texture_;
    std::string          uniform_name_;
    ImagePtr             image_;

    //! Overrides this to also set the label in the Ion texture.
    virtual void SetName_(const std::string &name) override;

    // XXXX
    void SetUniformName_(const std::string &name);
    void SetImage_(const ImagePtr &image);
    void SetSampler_(const SamplerPtr &sampler);

    friend class ::Input::Extractor;
};

}  // namespace Graph
