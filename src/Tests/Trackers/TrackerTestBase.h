#pragma once

#include "Tests/SceneTestBase.h"
#include "Util/Memory.h"

class Tracker;
DECL_SHARED_PTR(GenericWidget);
namespace SG {
DECL_SHARED_PTR(Scene);
DECL_SHARED_PTR(WindowCamera);
}

/// Base class for Tracker tests; it provides some conveniences to set up data.
///
/// \ingroup Tests
class TrackerTestBase : public SceneTestBase {
  protected:
    /// Sets up the given Tracker with a scene containing a GenericWidget
    /// covering the rectangle from (-1,-1) to (1,1) in the Z=0 plane. Returns
    /// the Scene.
    void InitTrackerScene(Tracker &tracker);

    /// Returns the Scene set up in InitTrackerScene().
    SG::ScenePtr GetScene() { return scene_; }

    /// Returns the WindowCamera from the Scene.
    SG::WindowCameraPtr GetWindowCamera() const;

    /// Returns the left GenericWidget from the Scene.
    GenericWidgetPtr GetLeftWidget() const;

    /// Returns the right GenericWidget from the Scene.
    GenericWidgetPtr GetRightWidget() const;

  private:
    SG::ScenePtr scene_;
};
