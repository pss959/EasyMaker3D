#include "Tests/Trackers/TrackerTestBase.h"

#include "Items/Controller.h"
#include "SG/Gantry.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/WindowCamera.h"
#include "Trackers/Tracker.h"
#include "Util/Assert.h"
#include "Widgets/GenericWidget.h"

// XXXX Move this to data file?
static Str input = R"(
Scene "TrackerScene" {
  CONSTANTS: [
    LIGHTING_PASS: "\"Lighting\"",
  ],
  gantry: Gantry {
    cameras: [
      WindowCamera {  # Needed to set up frustum
        position:  0 0 10,
        fov:       45,
        near:      .1,
        far:       100,
      },
    ],
  },
  root_node: Node "Root" {
    children: [
      <"nodes/templates/Controller.emd">,
      <"nodes/Controllers.emd">,
      GenericWidget "GW" {
        disabled_flags: "kRender",  # Just for interaction.
        shapes: [ Rectangle { size: 2 2 } ]
      },
    ],
  },
}
)";

void TrackerTestBase::InitTrackerScene(Tracker &tracker) {
    scene_ = ReadScene(input, false);   // Do NOT set up Ion.

    auto lc = SG::FindTypedNodeInScene<Controller>(*scene_, "LeftController");
    auto rc = SG::FindTypedNodeInScene<Controller>(*scene_, "RightController");

    tracker.Init(scene_, lc, rc);
}

SG::WindowCameraPtr TrackerTestBase::GetWindowCamera() const {
    ASSERT(scene_);
    SG::WindowCameraPtr wincam;
    for (auto &cam: scene_->GetGantry()->GetCameras()) {
        wincam = std::dynamic_pointer_cast<SG::WindowCamera>(cam);
        if (wincam)
            break;
    }
    ASSERT(wincam);
    return wincam;
}

WidgetPtr TrackerTestBase::GetWidget() const {
    return SG::FindTypedNodeInScene<Widget>(*scene_, "GW");
}
