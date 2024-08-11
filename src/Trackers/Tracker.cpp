//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Trackers/Tracker.h"

#include "Items/Controller.h"
#include "SG/CoordConv.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/KLog.h"
#include "Widgets/ClickableWidget.h"

Tracker::Tracker(Actuator actuator) : actuator_(actuator) {
    ASSERT(actuator != Actuator::kNone);
}

void Tracker::Init(const SG::ScenePtr &scene,
                   const ControllerPtr &lc, const ControllerPtr &rc) {
    ASSERT(scene);
    scene_ = scene;

    if (lc && rc) {
        left_controller_  = lc;
        right_controller_ = rc;

        // Find Controller paths.
        left_controller_path_  = SG::FindNodePathInScene(*scene_, *lc);
        right_controller_path_ = SG::FindNodePathInScene(*scene_, *rc);
    }
}

bool Tracker::IsLeft() const {  // LCOV_EXCL_START [cannot happen]
    ASSERTM(false, "Base Tracker::IsLeft() should not be called");
    return false;
}                               // LCOV_EXCL_STOP

Point3f Tracker::ToControllerCoords(const Point3f &p) const {
    const SG::NodePath &path = IsLeft() ?
        left_controller_path_ : right_controller_path_;
    ASSERT(! path.empty());
    return SG::CoordConv(path).RootToObject(p);
}

void Tracker::UpdateWidgetHovering(const WidgetPtr &old_widget,
                                   const WidgetPtr &new_widget) {
    ASSERT(old_widget != new_widget);
    if (old_widget && old_widget->IsHovering()) {
        KLOG('H', Util::EnumName(actuator_) << " Stop hovering "
             << old_widget->GetDesc());
        old_widget->StopHovering();
    }
    if (new_widget) {
        KLOG('H', Util::EnumName(actuator_) << " Start hovering "
             << new_widget->GetDesc());
        new_widget->StartHovering();
    }
}

float Tracker::GetMotionScale(const WidgetPtr &widget) {
    // See if the Widget is clickable and has any click observers. If so, scale
    // the motion by 1/2 so that more motion is required for a drag.
    if (auto clickable = std::dynamic_pointer_cast<ClickableWidget>(widget)) {
        if (clickable->GetClicked().GetObserverCount() > 0)
            return .5f;
    }
    return 1;
}
