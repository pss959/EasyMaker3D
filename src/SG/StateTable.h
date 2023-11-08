#pragma once

#include <ion/gfx/statetable.h>

#include "Math/Types.h"
#include "SG/Object.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(IonContext);
DECL_SHARED_PTR(StateTable);

/// A StateTable object wraps an Ion StateTable.
///
/// \ingroup SG
class StateTable : public Object {
  public:
    using CullFaceMode        = ion::gfx::StateTable::CullFaceMode;
    using BlendEquation       = ion::gfx::StateTable::BlendEquation;
    using BlendFunctionFactor = ion::gfx::StateTable::BlendFunctionFactor;

    /// Sets the line width field.
    void SetLineWidth(float width);

    /// Creates, stores, and returns Ion StateTable.
    ion::gfx::StateTablePtr SetUpIon(const IonContextPtr &ion_context);

    /// Returns the Ion StateTable for this instance. This will be null until
    /// SetUpIon() is called.
    ion::gfx::StateTablePtr GetIonStateTable() const {
        return ion_state_table_;
    }

    const Vector4f & GetClearColor()      const { return clear_color_;        }
    float            GetClearDepth()      const { return clear_depth_;        }
    int              GetClearStencil()    const { return clear_stencil_;      }
    float            GetLineWidth()       const { return line_width_;         }
    bool             IsBlendEnabled()     const { return blend_enabled_;      }
    bool             IsDepthTestEnabled() const { return depth_test_enabled_; }
    bool             IsCullFaceEnabled()  const { return cull_face_enabled_;  }
    CullFaceMode     GetCullFaceMode()    const { return cull_face_mode_;     }
    bool             IsCreateStencil()    const { return create_stencil_;     }
    bool             IsUseStencil()       const { return use_stencil_;        }
    bool             IsResetStencil()     const { return reset_stencil_;      }

    void             GetBlendEquations(BlendEquation &rgb,
                                       BlendEquation &alpha) const {
        rgb   = rgb_blend_equation_;
        alpha = alpha_blend_equation_;
    }
    void             GetBlendFunctions(BlendFunctionFactor &rgb_source,
                                       BlendFunctionFactor &rgb_dest,
                                       BlendFunctionFactor &alpha_source,
                                       BlendFunctionFactor &alpha_dest) const {
        rgb_source   = rgb_blend_source_factor_;
        rgb_dest     = rgb_blend_dest_factor_;
        alpha_source = alpha_blend_source_factor_;
        alpha_dest   = alpha_blend_dest_factor_;
    }

  protected:
    StateTable() {}

    virtual void AddFields() override;

  private:
    using IGS_ = ion::gfx::StateTable;  // Shorthand.

    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector4f>        clear_color_;
    Parser::TField<float>           clear_depth_;
    Parser::TField<int>             clear_stencil_;
    Parser::TField<float>           line_width_;
    Parser::TField<bool>            blend_enabled_;
    Parser::TField<bool>            depth_test_enabled_;
    Parser::TField<bool>            cull_face_enabled_;
    Parser::EnumField<CullFaceMode> cull_face_mode_;
    Parser::EnumField<BlendEquation> rgb_blend_equation_;
    Parser::EnumField<BlendEquation> alpha_blend_equation_;
    Parser::EnumField<BlendFunctionFactor> rgb_blend_source_factor_;
    Parser::EnumField<BlendFunctionFactor> rgb_blend_dest_factor_;
    Parser::EnumField<BlendFunctionFactor> alpha_blend_source_factor_;
    Parser::EnumField<BlendFunctionFactor> alpha_blend_dest_factor_;

    // These are used for stenciling.
    Parser::TField<bool>            create_stencil_;
    Parser::TField<bool>            use_stencil_;
    Parser::TField<bool>            reset_stencil_;
    ///@}

    ion::gfx::StateTablePtr ion_state_table_;

    friend class Parser::Registry;
};

}  // namespace SG
