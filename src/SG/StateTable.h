#pragma once

#include <ion/gfx/statetable.h>

#include "Math/Types.h"
#include "SG/Object.h"

namespace Parser { class Registry; }

namespace SG {

/// A StateTable object wraps an Ion StateTable.
class StateTable : public Object {
  public:
    typedef ion::gfx::StateTable::CullFaceMode CullFaceMode;

    virtual void AddFields() override;

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
    float            GetLineWidth()       const { return line_width_;         }
    bool             IsDepthTestEnabled() const { return depth_test_enabled_; }
    bool             IsCullFaceEnabled()  const { return cull_face_enabled_;  }
    CullFaceMode     GetCullFaceMode()    const { return cull_face_mode_;     }

  protected:
    StateTable() {}

  private:
    typedef ion::gfx::StateTable::Capability Capability_;  ///< Shorthand.

    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector4f>        clear_color_{"clear_color"};
    Parser::TField<float>           clear_depth_{"clear_depth"};
    Parser::TField<int>             clear_stencil_{"clear_stencil"};
    Parser::TField<float>           line_width_{"line_width"};
    Parser::TField<bool>            depth_test_enabled_{"depth_test_enabled"};
    Parser::TField<bool>            cull_face_enabled_{"cull_face_enabled"};
    Parser::EnumField<CullFaceMode> cull_face_mode_{"cull_face_mode"};

    // These are used for stenciling.
    Parser::TField<bool>            create_stencil_{"create_stencil"};
    Parser::TField<bool>            use_stencil_{"use_stencil"};
    ///@}

    ion::gfx::StateTablePtr ion_state_table_;

    friend class Parser::Registry;
};

}  // namespace SG
