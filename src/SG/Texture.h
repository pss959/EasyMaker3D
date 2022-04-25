#pragma once

#include <ion/gfx/texture.h>

#include "SG/Image.h"
#include "SG/Object.h"
#include "SG/Sampler.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(IonContext);
DECL_SHARED_PTR(Texture);

/// The Texture class wraps an Ion Texture.
///
/// \ingroup SG
class Texture : public Object {
  public:
    /// Creates, stores, and returns the Ion Texture.
    ion::gfx::TexturePtr SetUpIon(const IonContextPtr &ion_context);

    /// Returns the Ion Texture for this instance. This will be null
    /// until SetUpIon() is called.
    const ion::gfx::TexturePtr & GetIonTexture() const { return ion_texture_; }

    int                 GetCount()        const { return count_; }
    const std::string & GetUniformName()  const { return uniform_name_; }
    const ImagePtr    & GetImage()        const { return image_; }
    const SamplerPtr  & GetSampler()      const { return sampler_; }

  protected:
    Texture() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<int>          count_{"count", 1};
    Parser::TField<std::string>  uniform_name_{"uniform_name"};
    Parser::ObjectField<Image>   image_{"image"};
    Parser::ObjectField<Sampler> sampler_{"sampler"};
    ///@}

    ion::gfx::TexturePtr ion_texture_;

    friend class Parser::Registry;
};

}  // namespace SG
