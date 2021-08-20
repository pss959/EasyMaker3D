#pragma once

#include <ion/gfx/sampler.h>

#include "Parser/ObjectSpec.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"

namespace SG {

//! A Sampler object wraps an Ion sampler.
class Sampler : public Object {
  public:
    typedef ion::gfx::Sampler::WrapMode WrapMode;

    //! Returns the associated Ion sampler.
    const ion::gfx::SamplerPtr & GetIonSampler() const { return ion_sampler_; }

    WrapMode GetWrapSMode() const { return wrap_s_mode_; }
    WrapMode GetWrapTMode() const { return wrap_t_mode_; }

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    ion::gfx::SamplerPtr  ion_sampler_;  //! Associated Ion Sampler.

    //! \name Parsed Fields
    //!@{
    WrapMode wrap_s_mode_ = WrapMode::kRepeat;
    WrapMode wrap_t_mode_ = WrapMode::kRepeat;
    //!@}
};

}  // namespace SG
