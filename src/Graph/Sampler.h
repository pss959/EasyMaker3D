#pragma once

#include <ion/gfx/sampler.h>

#include "Graph/Object.h"

namespace Input { class Extractor; }

namespace Graph {

//! The Sampler class wraps an Ion Sampler.
//!
//! \ingroup Graph
class Sampler : public Object {
  public:
    //! Returns the associated Ion sampler.
    const ion::gfx::SamplerPtr &GetIonSampler() { return i_sampler_; }

  private:
    ion::gfx::SamplerPtr i_sampler_;

    friend class ::Input::Extractor;
};

}  // namespace Graph
