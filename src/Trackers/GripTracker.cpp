#include "Trackers/GripTracker.h"

#include <ion/math/vectorutils.h>

#include "App/ClickInfo.h"
#include "App/CoordConv.h"
#include "App/DragInfo.h"
#include "App/SceneContext.h"
#include "Base/Event.h"
#include "Items/Controller.h"
#include "Items/Grippable.h"
#include "Math/Linear.h"
#include "SG/Intersector.h"
#include "SG/Search.h"
#include "Util/General.h"
#include "Widgets/ClickableWidget.h"

GripTracker::GripTracker(Actuator actuator) : Tracker(actuator) {
    ASSERT(actuator == Actuator::kLeftGrip ||
           actuator == Actuator::kRightGrip);
}

void GripTracker::SetSceneContext(const SceneContextPtr &context) {
    Tracker::SetSceneContext(context);

    controller_ = GetActuator() == Actuator::kLeftGrip ?
        context->left_controller : context->right_controller;
    controller_path_ = SG::FindNodePathInScene(*context->scene, *controller_);
}

void GripTracker::UpdateHovering(const Event &event) {
    WidgetPtr widget;
    if (UpdateCurrentData_(event, widget) && widget != current_widget_) {
        UpdateWidgetHovering(current_widget_, widget);
        current_widget_ = widget;
    }
}

void GripTracker::StopHovering() {
    if (current_widget_)
        UpdateWidgetHovering(current_widget_, WidgetPtr());
}

bool GripTracker::IsActivation(const Event &event, WidgetPtr &widget) {
    if (event.flags.Has(Event::Flag::kButtonPress) &&
        event.device == GetDevice_() && event.button == Event::Button::kGrip) {
        UpdateCurrentData_(event, current_widget_);
        if (current_widget_) {
            if (current_widget_->IsHovering())
                current_widget_->SetHovering(false);
            current_widget_->SetActive(true);
        }
        activation_data_ = current_data_;
        UpdateControllers_(true);
        widget = current_widget_;
        return true;
    }
    return false;
}

bool GripTracker::IsDeactivation(const Event &event, WidgetPtr &widget) {
    if (event.flags.Has(Event::Flag::kButtonRelease) &&
        event.device == GetDevice_() && event.button == Event::Button::kGrip) {
        if (current_widget_)
            current_widget_->SetActive(false);
        UpdateControllers_(false);
        UpdateCurrentData_(event, widget);
        return true;
    }
    return false;
}

bool GripTracker::MovedEnoughForDrag(const Event &event) {
    // Get the grip data; no need for GripInfo.
    Data_ data;
    if (! GetGripData_(event, false, data))
        return false;

    /// Minimum angle between two ray directions to be considered enough for a
    // drag.
    const Anglef kMinRayAngle = Anglef::FromDegrees(10);

    /// Minimum world-space distance for a controller to move to be considered
    // a potential grip drag operation.
    const float  kMinDragDistance = .04f;

    // Clickable Widgets require extra motion to start a drag, since small
    // movements should not interfere with a click.
    const bool is_clickable =
        Util::CastToDerived<ClickableWidget>(current_widget_).get();
    // Use half the threshhold if the widget is not also clickable.
    const float scale = is_clickable ? 1.f : .5f;

    // Check for position change and then rotation change.
    const Point3f   &p0 = activation_data_.position;
    const Point3f   &p1 = data.position;
    const Rotationf &r0 = activation_data_.orientation;
    const Rotationf &r1 = data.orientation;
    return ion::math::Distance(p0, p1) > scale * kMinDragDistance ||
        AbsAngle(RotationAngle(RotationDifference(r0, r1))) >
        scale * kMinRayAngle;
}

void GripTracker::FillActivationDragInfo(DragInfo &info) {
    info.trigger          = Trigger::kGrip;
    info.grip_position    = activation_data_.position;
    info.grip_orientation = activation_data_.orientation;
}

void GripTracker::FillEventDragInfo(const Event &event, DragInfo &info) {
    Data_ data;
    if (GetGripData_(event, false, data)) {
        info.trigger          = Trigger::kGrip;
        info.grip_position    = data.position;
        info.grip_orientation = data.orientation;
    }
}

void GripTracker::FillClickInfo(ClickInfo &info) {
    info.device = GetDevice_();
    info.widget = Util::CastToDerived<ClickableWidget>(current_widget_).get();
}

void GripTracker::Reset() {
    current_data_.info        = Grippable::GripInfo();
    current_data_.position    = Point3f::Zero();
    current_data_.orientation = Rotationf::Identity();
    activation_data_ = current_data_;
    current_widget_.reset();
}

Event::Device GripTracker::GetDevice_() const {
    return GetActuator() == Actuator::kLeftGrip ?
        Event::Device::kLeftController : Event::Device::kRightController;
}

bool GripTracker::UpdateCurrentData_(const Event &event, WidgetPtr &widget) {
    // Get the grip data, including the full GripInfo.
    Data_ data;
    if (! GetGripData_(event, true, data))
        return false;

    widget = data.info.widget;
    current_data_ = data;

    // Update the Controller grip hover.
    if (widget && ! grippable_path_.empty()) {
        const Point3f world_pt =
            CoordConv(grippable_path_).ObjectToRoot(data.info.target_point);
        const Point3f pt =
            CoordConv(controller_path_).RootToObject(world_pt);
        controller_->ShowGripHover(true, pt, data.info.color);
    }
    else {
        controller_->ShowGripHover(false, Point3f::Zero(), data.info.color);
    }
    return true;
}

bool GripTracker::GetGripData_(const Event &event, bool add_info,
                               Data_ &data) const {
    if (event.flags.Has(Event::Flag::kPosition3D) &&
        event.flags.Has(Event::Flag::kOrientation) &&
        ((GetActuator() == Actuator::kLeftGrip &&
          event.device == Event::Device::kLeftController) ||
         (GetActuator() == Actuator::kRightGrip &&
          event.device == Event::Device::kRightController))) {

        data.position    = event.position3D;
        data.orientation = event.orientation;

        if (add_info) {
            Grippable::GripInfo &info = data.info;
            info = Grippable::GripInfo();
            info.event      = event;
            info.controller = controller_;
            info.guide_direction =
                event.orientation * controller_->GetGuideDirection();

            // Let the Grippable (if any) fill in the rest of the GripInfo.
            if (grippable_)
                grippable_->UpdateGripInfo(info);
        }

        return true;
    }
    return false;
}

void GripTracker::UpdateControllers_(bool is_active) {
    const auto &context = GetContext();
    controller_->SetTriggerMode(Trigger::kGrip, is_active);
    const auto &other_controller = controller_ == context.left_controller ?
        context.right_controller : context.left_controller;
    other_controller->ShowAll(! is_active);

    if (grippable_)
        grippable_->ActivateGrip(controller_->GetHand(), is_active);
}
