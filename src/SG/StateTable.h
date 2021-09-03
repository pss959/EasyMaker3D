#pragma once

#include <ion/gfx/statetable.h>

#include "Math/Types.h"
#include "SG/Object.h"

namespace SG {

//! A StateTable object wraps an Ion StateTable.
class StateTable : public Object {
  public:
    typedef ion::gfx::StateTable::CullFaceMode CullFaceMode;

    virtual void AddFields() override;

    //! Returns the associated Ion stateTable.
    const ion::gfx::StateTablePtr & GetIonStateTable() const {
        return ion_state_table_;
    }

    const Vector4f & GetClearColor()      const { return clear_color_;        }
    float            GetClearDepth()      const { return clear_depth_;        }
    float            GetLineWidth()       const { return line_width_;         }
    bool             IsDepthTestEnabled() const { return depth_test_enabled_; }
    bool             IsCullFaceEnabled()  const { return cull_face_enabled_;  }
    CullFaceMode     GetCullFaceMode()    const { return cull_face_mode_;     }

    virtual void SetUpIon(IonContext &context) override;

  private:
    typedef ion::gfx::StateTable::Capability Capability_;  //!< Shorthand.

    ion::gfx::StateTablePtr  ion_state_table_;  //!< Associated Ion StateTable.

    //! \name Parsed Fields
    //!@{
    Parser::TField<Vector4f>        clear_color_{"clear_color"};
    Parser::TField<float>           clear_depth_{"clear_depth"};
    Parser::TField<float>           line_width_{"line_width"};
    Parser::TField<bool>            depth_test_enabled_{"depth_test_enabled"};
    Parser::TField<bool>            cull_face_enabled_{"cull_face_enabled"};
    Parser::EnumField<CullFaceMode> cull_face_mode_{"cull_face_mode"};
    //!@}
};

}  // namespace SG
