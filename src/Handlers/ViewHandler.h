#pragma once

#include "Handlers/Handler.h"
#include "Math/Types.h"
#include "SG/Typedefs.h"

/// ViewHandler is a derived Handler that processes events to update the
/// current view.
/// |ingroup Handlers
class ViewHandler : public Handler {
  public:
    /// Sets the WindowCamera to update.
    void SetCamera(const SG::WindowCameraPtr &camera) { camera_ = camera; }

    // ------------------------------------------------------------------------
    // Handler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

    // ------------------------------------------------------------------------
    // Other public interface.
    // ------------------------------------------------------------------------

    /// Resets the view to the default view.
    void ResetView();

  private:
    /// WindowCamera to update.
    SG::WindowCameraPtr camera_;

    /// Set to true while in the middle of a view-changing action.
    bool is_changing_view_ = false;

    /// Current extra rotation applied to the camera.
    Rotationf rotation_;

    /// Starting 2D window position for a window cursor drag.
    Point2f start_pos_;

    /// Starting camera rotation for a window cursor drag.
    Rotationf start_rot_;
};
