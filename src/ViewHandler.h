#pragma once

#include <ion/math/rotation.h>
#include <ion/math/vector.h>

#include "Interfaces/IHandler.h"

class  IScene;
struct View;

//! ViewHandler is a derived IHandler that processes events to update the
//! current view.
class ViewHandler : public IHandler {
  public:
    //! The constructor is passed the View to update.
    ViewHandler(View &view);
    virtual ~ViewHandler();

    virtual const char * GetClassName() const override { return "ViewHandler"; }

    // ------------------------------------------------------------------------
    // IHandler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    //! View to update.
    View &view_;

    //! Set to true while in the middle of a view-changing action.
    bool is_changing_view_ = false;

    //! Current extra rotation applied to the camera.
    ion::math::Rotationf rotation_;

    //! Starting 2D window position for a window cursor drag.
    ion::math::Point2f start_pos_;

    //! Starting camera rotation for a window cursor drag.
    ion::math::Rotationf start_rot_;
};
