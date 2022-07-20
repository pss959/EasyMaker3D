#include "Trackers/Tracker.h"

#include "App/CoordConv.h"
#include "App/SceneContext.h"
#include "Items/Controller.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Widgets/ClickableWidget.h"

// ----------------------------------------------------------------------------
// Tracker functions.
// ----------------------------------------------------------------------------

Tracker::Tracker(Actuator actuator) : actuator_(actuator) {
    ASSERT(actuator != Actuator::kNone);
}

void Tracker::SetSceneContext(const SceneContextPtr &context) {
    context_ = context;
}

SceneContext & Tracker::GetContext() const {
    ASSERT(context_);
    return *context_;
}

void Tracker::UpdateWidgetHovering(const WidgetPtr &old_widget,
                                   const WidgetPtr &new_widget) {
    ASSERT(old_widget != new_widget);
    if (old_widget && old_widget->IsHovering())
        old_widget->SetHovering(false);
    if (new_widget)
        new_widget->SetHovering(true);
}

float Tracker::GetMotionScale(const WidgetPtr &widget) {
    // See if the Widget is clickable and has any click observers. If so, scale
    // the motion by 1/2 so that more motion is required for a drag.
    if (auto clickable = Util::CastToDerived<ClickableWidget>(widget)) {
        if (clickable->GetClicked().GetObserverCount() > 0)
            return .5f;
    }
    return 1;
}

// ----------------------------------------------------------------------------
// Tracker::ControllerData functions.
// ----------------------------------------------------------------------------

void Tracker::ControllerData::Init(const SceneContext &context, Hand hand) {
    controller_ = hand == Hand::kLeft ?
        context.left_controller : context.right_controller;
    other_controller_ = hand == Hand::kLeft ?
        context.right_controller : context.left_controller;
    path_ = SG::FindNodePathInScene(*context.scene, *controller_);
}

Controller & Tracker::ControllerData::GetController() const {
    ASSERT(controller_);
    return *controller_;
}

Controller & Tracker::ControllerData::GetOtherController() const {
    ASSERT(other_controller_);
    return *other_controller_;
}

Point3f Tracker::ControllerData::ToControllerCoords(const Point3f &p) const {
    ASSERT(! path_.empty());
    return CoordConv(path_).RootToObject(p);
}
