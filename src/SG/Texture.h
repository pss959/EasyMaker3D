//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

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
    ~Texture();

    /// Creates, stores, and returns the Ion Texture.
    ion::gfx::TexturePtr SetUpIon(const IonContextPtr &ion_context);

    /// Returns the Ion Texture for this instance. This will be null
    /// until SetUpIon() is called.
    const ion::gfx::TexturePtr & GetIonTexture() const { return ion_texture_; }

    int                GetCount()        const { return count_; }
    const Str &        GetUniformName()  const { return uniform_name_; }
    const ImagePtr   & GetImage()        const { return image_; }
    const SamplerPtr & GetSampler()      const { return sampler_; }

  protected:
    Texture() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

    /// Redefines this to detect changes to the Image.
    virtual bool ProcessChange(Change change, const Object &obj) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<int>          count_;
    Parser::TField<Str>          uniform_name_;
    Parser::ObjectField<Image>   image_;
    Parser::ObjectField<Sampler> sampler_;
    ///@}

    ion::gfx::TexturePtr ion_texture_;

    friend class Parser::Registry;
};

}  // namespace SG
