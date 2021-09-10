#pragma once

#include "Interfaces/IHandler.h"
#include "Math/Types.h"
#include "SG/Typedefs.h"

//! ViewHandler is a derived IHandler that processes events to update the
//! current view.
class ViewHandler : public IHandler {
  public:
    //! The constructor is passed the WindowCamera to update.
    ViewHandler(const SG::WindowCameraPtr &camera);
    virtual ~ViewHandler();

    virtual const char * GetClassName() const override { return "ViewHandler"; }

    // ------------------------------------------------------------------------
    // IHandler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

    // ------------------------------------------------------------------------
    // Other public interface.
    // ------------------------------------------------------------------------

    //! Resets the view to the default view.
    void ResetView();

  private:
    //! WindowCamera to update.
    SG::WindowCameraPtr camera_;

    //! Set to true while in the middle of a view-changing action.
    bool is_changing_view_ = false;

    //! Current extra rotation applied to the camera.
    Rotationf rotation_;

    //! Starting 2D window position for a window cursor drag.
    Point2f start_pos_;

    //! Starting camera rotation for a window cursor drag.
    Rotationf start_rot_;
};
