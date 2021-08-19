#pragma once

#include <ion/gfx/statetable.h>

#include "NParser/ObjectSpec.h"
#include "SG/Math.h"
#include "SG/Object.h"

namespace SG {

//! A StateTable object wraps an Ion StateTable.
class StateTable : public Object {
  public:
    //! Returns the associated Ion stateTable.
    const ion::gfx::StateTablePtr & GetIonStateTable() const {
        return ion_state_table_;
    }

    // XXXX
    const Vector4f & GetClearColor()      const { return clear_color_;        }
    bool             IsDepthTestEnabled() const { return depth_test_enabled_; }
    bool             IsCullFaceEnabled()  const { return cull_face_enabled_;  }

    virtual void SetUpIon(IonContext &context) override;

    static NParser::ObjectSpec GetObjectSpec();

  private:
    typedef ion::gfx::StateTable::Capability Capability_;  //!< Shorthand.

    ion::gfx::StateTablePtr  ion_state_table_;  //! Associated Ion StateTable.

    // Parsed fields.
    Vector4f clear_color_;
    bool     depth_test_enabled_ = false;
    bool     cull_face_enabled_  = false;
};

}  // namespace SG
