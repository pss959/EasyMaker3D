#pragma once

#include <ion/math/vector.h>

#include "Interfaces/IHandler.h"

//! ViewHandler is a derived IHandler that processes events to update the
//! current view.
class ViewHandler : public IHandler {
  public:
    ViewHandler();
    virtual ~ViewHandler();

    virtual const char * GetClassName() const override { return "ViewHandler"; }

    // ------------------------------------------------------------------------
    // IHandler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    //! Set to true while in the middle of a view-changing action.
    bool is_changing_view_ = false;

    //! Starting 2D window position for a window cursor drag.
    ion::math::Point2f start_pos_;
};
