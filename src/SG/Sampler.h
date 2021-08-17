#pragma once

#include <ion/gfx/sampler.h>

#include "NParser/FieldSpec.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"

namespace SG {

//! A Sampler object wraps an Ion sampler.
class Sampler : public Object {
  public:
    typedef ion::gfx::Sampler::WrapMode WrapMode;

    //! Returns the associated Ion sampler.
    const ion::gfx::SamplerPtr & GetIonSampler() const { return ion_sampler_; }

    // XXXX
    WrapMode GetWrapSMode() const { return wrap_s_mode_; }
    WrapMode GetWrapTMode() const { return wrap_t_mode_; }

  private:
    ion::gfx::SamplerPtr  ion_sampler_;  //! Associated Ion Sampler.

    // Parsed fields.
    WrapMode wrap_s_mode_;
    WrapMode wrap_t_mode_;

    //! Redefines this to set up the Ion Sampler.
    virtual void Finalize() override;

    static std::vector<NParser::FieldSpec> GetFieldSpecs_();
};

}  // namespace SG
