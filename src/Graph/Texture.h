#pragma once

#include <ion/gfx/texture.h>

#include "Graph/Object.h"

namespace Input { class Extractor; }

namespace Graph {

//! The Texture class wraps an Ion Texture.
//!
//! \ingroup Graph
class Texture : public Object {
  public:
    //! Returns the associated Ion texture.
    const ion::gfx::TexturePtr &GetIonTexture() { return i_texture_; }

  private:
    ion::gfx::TexturePtr i_texture_;

    friend class ::Input::Extractor;
};

}  // namespace Graph
