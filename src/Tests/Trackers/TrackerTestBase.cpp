#include "Tests/Trackers/TrackerTestBase.h"

#include "Items/Controller.h"
#include "SG/Gantry.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/WindowCamera.h"
#include "Trackers/Tracker.h"
#include "Util/Assert.h"
#include "Widgets/GenericWidget.h"

static const Str kContents = R"(
    children: [
      <"nodes/templates/Controller.emd">,
      <"nodes/Controllers.emd">,
      GenericWidget "LeftGW" {
        disabled_flags: "kRender",  # Just for interaction.
        translation: -1 0 0,
        shapes: [ Rectangle { size: 2 2 } ]
      },
      GenericWidget "RightGW" {
        disabled_flags: "kRender",  # Just for interaction.
        translation: 1 0 0,
        shapes: [ Rectangle { size: 2 2 } ]
      },
    ],
)";

void TrackerTestBase::InitTrackerScene(Tracker &tracker) {
    scene_ = ReadRealScene(kContents);

    auto lc = SG::FindTypedNodeInScene<Controller>(*scene_, "LeftController");
    auto rc = SG::FindTypedNodeInScene<Controller>(*scene_, "RightController");

    // Set up the WindowCamera with useful settings.
    auto wincam = GetWindowCamera();
    wincam->SetPosition(Point3f(0, 0, 10.1f));
    wincam->SetOrientation(Rotationf::Identity());
    wincam->SetFOV(Anglef::FromDegrees(45));
    wincam->SetNearAndFar(.1f, 100);

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

GenericWidgetPtr TrackerTestBase::GetLeftWidget() const {
    return SG::FindTypedNodeInScene<GenericWidget>(*scene_, "LeftGW");
}

GenericWidgetPtr TrackerTestBase::GetRightWidget() const {
    return SG::FindTypedNodeInScene<GenericWidget>(*scene_, "RightGW");
}
