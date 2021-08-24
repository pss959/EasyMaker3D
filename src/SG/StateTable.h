#pragma once

#include <ion/gfx/statetable.h>

#include "Parser/ObjectSpec.h"
#include "SG/Math.h"
#include "SG/Object.h"

namespace SG {

//! A StateTable object wraps an Ion StateTable.
class StateTable : public Object {
  public:
    typedef ion::gfx::StateTable::CullFaceMode CullFaceMode;

    //! Returns the associated Ion stateTable.
    const ion::gfx::StateTablePtr & GetIonStateTable() const {
        return ion_state_table_;
    }

    const Vector4f & GetClearColor()      const { return clear_color_;        }
    float            GetClearDepth()      const { return clear_depth_;        }
    bool             IsDepthTestEnabled() const { return depth_test_enabled_; }
    bool             IsCullFaceEnabled()  const { return cull_face_enabled_;  }
    CullFaceMode     GetCullFaceMode()    const { return cull_face_mode_;     }

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    typedef ion::gfx::StateTable::Capability Capability_;  //!< Shorthand.

    ion::gfx::StateTablePtr  ion_state_table_;  //!< Associated Ion StateTable.

    //! \name Parsed Fields
    //!@{
    Vector4f clear_color_{0, 0, 0, 0};
    float    clear_depth_        = 1.f;
    bool     depth_test_enabled_ = false;
    bool     cull_face_enabled_  = false;
    CullFaceMode cull_face_mode_ = CullFaceMode::kCullBack;
    //!@}
};

}  // namespace SG
