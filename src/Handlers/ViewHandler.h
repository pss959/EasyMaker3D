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

    /// Indicates whether the position of the viewer can change to maintain a
    /// consistent rotation center (false) or just rotate about the camera
    /// position (true). The default is true.
    void SetFixedCameraPosition(bool is_fixed) { is_fixed_pos_ = is_fixed; }

    // ------------------------------------------------------------------------
    // Handler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

    // ------------------------------------------------------------------------
    // Other public interface.
    // ------------------------------------------------------------------------

    /// Sets the center of view rotation, which is the origin by default.
    void SetRotationCenter(const Point3f &center);

    /// Returns the current center of rotation.
    const Point3f & GetRotationCenter() const { return rot_center_; }

    /// Resets the view to the default view.
    void ResetView();

  private:
    /// WindowCamera to update.
    SG::WindowCameraPtr camera_;

    /// True if the camera position should not change. The default is true.
    bool is_fixed_pos_ = true;

    /// Set to true while in the middle of a view-changing action.
    bool is_changing_view_ = false;

    /// Center point for rotation.
    Point3f rot_center_{0, 0, 0};

    /// Current extra rotation applied to the camera.
    Rotationf rotation_;

    /// Starting 2D window position for a window cursor drag.
    Point2f start_pos_;

    /// Starting camera rotation for a window cursor drag.
    Rotationf start_rot_;

    // Modifies the camera position if the center of rotation is not the
    // origin.
    void SetPosition_();
};

typedef std::shared_ptr<ViewHandler> ViewHandlerPtr;
