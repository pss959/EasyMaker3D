#pragma once

#include <ion/gfx/sampler.h>

#include "SG/Object.h"
#include "SG/Typedefs.h"

namespace Parser { class Registry; }

namespace SG {

/// A Sampler object wraps an Ion sampler.
class Sampler : public Object {
  public:
    typedef ion::gfx::Sampler::CompareFunction CompareFunction;
    typedef ion::gfx::Sampler::CompareMode     CompareMode;
    typedef ion::gfx::Sampler::FilterMode      FilterMode;
    typedef ion::gfx::Sampler::WrapMode        WrapMode;

    virtual void AddFields() override;

    /// Returns the associated Ion sampler, creating it first if necessary.
    const ion::gfx::SamplerPtr & GetIonSampler();

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

  protected:
    Sampler() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<bool>               auto_mipmaps_{"auto_mipmaps"};
    Parser::EnumField<CompareMode>     compare_mode_{
        "compare_mode", CompareMode::kNone};
    Parser::EnumField<CompareFunction> compare_function_{
        "compare_function", CompareFunction::kLess};
    Parser::EnumField<FilterMode>      min_filter_{
        "min_filter", FilterMode::kNearest};
    Parser::EnumField<FilterMode>      mag_filter_{
        "mag_filter", FilterMode::kNearest};
    Parser::EnumField<WrapMode>        wrap_r_mode_{
        "wrap_r_mode", WrapMode::kRepeat};
    Parser::EnumField<WrapMode>        wrap_s_mode_{
        "wrap_s_mode", WrapMode::kRepeat};
    Parser::EnumField<WrapMode>        wrap_t_mode_{
        "wrap_t_mode", WrapMode::kRepeat};
    Parser::TField<float>              max_anisotropy_{"max_anisotropy", 1.f};
    Parser::TField<float>              min_lod_{"min_lod", -1000.f};
    Parser::TField<float>              max_lod_{"max_lod",  1000.f};
    ///@}

    ion::gfx::SamplerPtr  ion_sampler_;  /// Associated Ion Sampler.

    friend class Parser::Registry;
};

}  // namespace SG
