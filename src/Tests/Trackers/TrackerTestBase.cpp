//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

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
    if (! scene_) {
        scene_ = ReadRealScene(kContents);

        // Set up the WindowCamera with useful settings.
        auto wincam = GetWindowCamera();
        wincam->SetPosition(Point3f(0, 0, 10.1f));
        wincam->SetOrientation(Rotationf::Identity());
        wincam->SetFOV(Anglef::FromDegrees(45));
        wincam->SetNearAndFar(.1f, 100);
    }

    auto lc = SG::FindTypedNodeInScene<Controller>(*scene_, "LeftController");
    auto rc = SG::FindTypedNodeInScene<Controller>(*scene_, "RightController");
    tracker.Init(scene_, lc, rc);
}

SG::WindowCameraPtr TrackerTestBase::GetWindowCamera() const {
    ASSERT(scene_);
    auto wincam = scene_->GetTypedCamera<SG::WindowCamera>();
    ASSERT(wincam);
    return wincam;
}

GenericWidgetPtr TrackerTestBase::GetLeftWidget() const {
    return SG::FindTypedNodeInScene<GenericWidget>(*scene_, "LeftGW");
}

GenericWidgetPtr TrackerTestBase::GetRightWidget() const {
    return SG::FindTypedNodeInScene<GenericWidget>(*scene_, "RightGW");
}
