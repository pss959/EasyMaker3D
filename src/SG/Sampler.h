#pragma once

#include <ion/gfx/sampler.h>

#include "Parser/ObjectSpec.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"

namespace SG {

//! A Sampler object wraps an Ion sampler.
class Sampler : public Object {
  public:
    typedef ion::gfx::Sampler::CompareFunction CompareFunction;
    typedef ion::gfx::Sampler::CompareMode     CompareMode;
    typedef ion::gfx::Sampler::FilterMode      FilterMode;
    typedef ion::gfx::Sampler::WrapMode        WrapMode;

    //! Returns the associated Ion sampler.
    const ion::gfx::SamplerPtr & GetIonSampler() const { return ion_sampler_; }

    bool            IsAutoMipmapsEnabled() const { return auto_mipmaps_;     }
    CompareMode     GetCompareMode()       const { return compare_mode_;     }
    CompareFunction GetCompareFunction()   const { return compare_function_; }
    FilterMode      GetMinFilter()         const { return min_filter_;       }
    FilterMode      GetMagFilter()         const { return mag_filter_;       }
    WrapMode        GetWrapRMode()         const { return wrap_r_mode_;      }
    WrapMode        GetWrapSMode()         const { return wrap_s_mode_;      }
    WrapMode        GetWrapTMode()         const { return wrap_t_mode_;      }
    float           GetMaxAnisotropy()     const { return max_anisotropy_;   }
    float           GetMinLOD()            const { return min_lod_;          }
    float           GetMaxLOD()            const { return max_lod_;          }

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    ion::gfx::SamplerPtr  ion_sampler_;  //! Associated Ion Sampler.

    //! \name Parsed Fields
    //!@{
    bool            auto_mipmaps_     = false;
    CompareMode     compare_mode_     = CompareMode::kNone;
    CompareFunction compare_function_ = CompareFunction::kLess;
    FilterMode      min_filter_       = FilterMode::kNearest;
    FilterMode      mag_filter_       = FilterMode::kNearest;
    WrapMode        wrap_r_mode_      = WrapMode::kRepeat;
    WrapMode        wrap_s_mode_      = WrapMode::kRepeat;
    WrapMode        wrap_t_mode_      = WrapMode::kRepeat;
    float           max_anisotropy_   = 1.f;
    float           min_lod_          = -1000.f;
    float           max_lod_          =  1000.f;
    //!@}
};

}  // namespace SG
