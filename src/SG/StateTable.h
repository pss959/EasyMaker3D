#pragma once

#include <ion/gfx/statetable.h>

#include "Math/Types.h"
#include "Memory.h"
#include "SG/Object.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(StateTable);

/// A StateTable object wraps an Ion StateTable.
class StateTable : public Object {
  public:
    typedef ion::gfx::StateTable::CullFaceMode        CullFaceMode;
    typedef ion::gfx::StateTable::BlendEquation       BlendEquation;
    typedef ion::gfx::StateTable::BlendFunctionFactor BlendFunctionFactor;

    /// Sets the line width field.
    void SetLineWidth(float width);

    /// Creates, stores, and returns Ion StateTable.
    ion::gfx::StateTablePtr SetUpIon();

    /// Returns the Ion StateTable for this instance. This will be null until
    /// SetUpIon() is called.
    ion::gfx::StateTablePtr GetIonStateTable() const {
        return ion_state_table_;
    }

    const Vector4f & GetClearColor()      const { return clear_color_;        }
    float            GetClearDepth()      const { return clear_depth_;        }
    int              GetClearStencil()    const { return clear_stencil_;      }

    float            GetLineWidth()       const { return line_width_;         }

    bool             IsCullFaceEnabled()  const { return cull_face_enabled_;  }
    CullFaceMode     GetCullFaceMode()    const { return cull_face_mode_;     }

    bool             IsBlendEnabled()     const { return blend_enabled_; }
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
    typedef ion::gfx::StateTable::Capability Capability_;  ///< Shorthand.

    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector4f>        clear_color_{"clear_color"};
    Parser::TField<float>           clear_depth_{"clear_depth", 0};
    Parser::TField<int>             clear_stencil_{"clear_stencil", 0};
    Parser::TField<float>           line_width_{"line_width", 1};
    Parser::TField<bool>            blend_enabled_{"blend_enabled", false};
    Parser::TField<bool>            depth_test_enabled_{"depth_test_enabled",
                                                        false};
    Parser::TField<bool>            cull_face_enabled_{"cull_face_enabled",
                                                       false};
    Parser::EnumField<CullFaceMode> cull_face_mode_{
        "cull_face_mode", ion::gfx::StateTable::kCullBack};
    Parser::EnumField<BlendEquation> rgb_blend_equation_{
        "rgb_blend_equation", ion::gfx::StateTable::kAdd};
    Parser::EnumField<BlendEquation> alpha_blend_equation_{
        "alpha_blend_equation", ion::gfx::StateTable::kAdd};
    Parser::EnumField<BlendFunctionFactor> rgb_blend_source_factor_{
        "rgb_blend_source_factor", ion::gfx::StateTable::kOne};
    Parser::EnumField<BlendFunctionFactor> rgb_blend_dest_factor_{
        "rgb_blend_dest_factor", ion::gfx::StateTable::kZero};
    Parser::EnumField<BlendFunctionFactor> alpha_blend_source_factor_{
        "alpha_blend_source_factor", ion::gfx::StateTable::kOne};
    Parser::EnumField<BlendFunctionFactor> alpha_blend_dest_factor_{
        "alpha_blend_dest_factor", ion::gfx::StateTable::kZero};

    // These are used for stenciling.
    Parser::TField<bool>            create_stencil_{"create_stencil", false};
    Parser::TField<bool>            use_stencil_{"use_stencil", false};
    Parser::TField<bool>            reset_stencil_{"reset_stencil", false};
    ///@}

    ion::gfx::StateTablePtr ion_state_table_;

    friend class Parser::Registry;
};

}  // namespace SG
