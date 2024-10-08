//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <ion/gfx/sampler.h>

#include "SG/Object.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(IonContext);
DECL_SHARED_PTR(Sampler);

/// A Sampler object wraps an Ion sampler.
///
/// \ingroup SG
class Sampler : public Object {
  public:
    using CompareFunction = ion::gfx::Sampler::CompareFunction;
    using CompareMode     = ion::gfx::Sampler::CompareMode;
    using FilterMode      = ion::gfx::Sampler::FilterMode;
    using WrapMode        = ion::gfx::Sampler::WrapMode;

    /// \name Field Query Functions
    ///@{
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
    ///@}

    /// Creates, stores, and returns the Ion Sampler.
    ion::gfx::SamplerPtr SetUpIon(const IonContextPtr &ion_context);

    /// Returns the Ion Sampler for this instance. This will be null
    /// until SetUpIon() is called.
    const ion::gfx::SamplerPtr & GetIonSampler() const { return ion_sampler_; }

  protected:
    Sampler() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<bool>               auto_mipmaps_;
    Parser::EnumField<CompareMode>     compare_mode_;
    Parser::EnumField<CompareFunction> compare_function_;
    Parser::EnumField<FilterMode>      min_filter_;
    Parser::EnumField<FilterMode>      mag_filter_;
    Parser::EnumField<WrapMode>        wrap_r_mode_;
    Parser::EnumField<WrapMode>        wrap_s_mode_;
    Parser::EnumField<WrapMode>        wrap_t_mode_;
    Parser::TField<float>              max_anisotropy_;
    Parser::TField<float>              min_lod_;
    Parser::TField<float>              max_lod_;
    ///@}

    ion::gfx::SamplerPtr ion_sampler_;  ///< Associated Ion Sampler.

    friend class Parser::Registry;
};

}  // namespace SG
