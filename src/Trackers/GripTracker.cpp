#include "Trackers/GripTracker.h"

#include <ion/math/vectorutils.h>

#include "Base/Event.h"
#include "Items/Controller.h"
#include "Items/Grippable.h"
#include "Math/Linear.h"
#include "Place/ClickInfo.h"
#include "Place/DragInfo.h"
#include "SG/CoordConv.h"
#include "SG/Intersector.h"
#include "SG/Search.h"
#include "Util/General.h"
#include "Util/Tuning.h"
#include "Widgets/ClickableWidget.h"

GripTracker::GripTracker(Actuator actuator) : Tracker(actuator) {
    ASSERT(actuator == Actuator::kLeftGrip ||
           actuator == Actuator::kRightGrip);
}

Event::Device GripTracker::GetDevice() const {
    return IsLeft() ?
        Event::Device::kLeftController : Event::Device::kRightController;
}

void GripTracker::UpdateHovering(const Event &event) {
    WidgetPtr widget;
    if (UpdateCurrentData_(event, widget) && widget != current_widget_&&
        widget->IsInteractionEnabled()) {
        UpdateWidgetHovering(current_widget_, widget);
        current_widget_ = widget;
    }
    if (widget)
        widget->UpdateHoverPoint(current_data_.position);
}

void GripTracker::StopHovering() {
    if (current_widget_)
        UpdateWidgetHovering(current_widget_, WidgetPtr());
    GetController()->ShowGripHover(false, Point3f::Zero(), Color::White());
}

bool GripTracker::IsActivation(const Event &event, WidgetPtr &widget) {
    if (event.flags.Has(Event::Flag::kButtonPress) &&
        event.device == GetDevice() && event.button == Event::Button::kGrip) {
        UpdateCurrentData_(event, current_widget_);
        if (current_widget_) {
            if (current_widget_->IsHovering())
                current_widget_->StopHovering();
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
        event.device == GetDevice() && event.button == Event::Button::kGrip) {
        if (current_widget_)
            current_widget_->SetActive(false);
        UpdateControllers_(false);
        UpdateCurrentData_(event, widget);
        return true;
    }
    return false;
}

float GripTracker::GetClickTimeout() const {
    return TK::kGripClickTimeout;
}

bool GripTracker::MovedEnoughForDrag(const Event &event) {
    // Get the grip data; no need for GripInfo.
    Data_ data;
    if (! GetGripData_(event, false, data))
        return false;

    const float motion_scale = GetMotionScale(current_widget_);

    // Check for position change and then rotation change.
    const Point3f &p0 = activation_data_.position;
    const Point3f &p1 = data.position;
    const float distance = motion_scale * ion::math::Distance(p0, p1);
    if (distance > TK::kMinGripControllerDistance)
        return true;

    const Rotationf &r0 = activation_data_.orientation;
    const Rotationf &r1 = data.orientation;
    const auto angle = AbsAngle(RotationAngle(RotationDifference(r0, r1)));
    return motion_scale * angle > TK::kMinGripOrientationAngleChange;
}

void GripTracker::FillActivationDragInfo(DragInfo &info) {
    info.trigger              = Trigger::kGrip;
    info.grip_guide_direction = GetController()->GetGuideDirection();
    info.grip_position        = activation_data_.position;
    info.grip_orientation     = activation_data_.orientation;
}

void GripTracker::FillEventDragInfo(const Event &event, DragInfo &info) {
    Data_ data;
    if (GetGripData_(event, false, data)) {
        info.trigger              = Trigger::kGrip;
        info.grip_guide_direction = GetController()->GetGuideDirection();
        info.grip_position        = data.position;
        info.grip_orientation     = data.orientation;
    }
}

void GripTracker::FillClickInfo(ClickInfo &info) {
    info.device = GetDevice();
    info.widget = Util::CastToDerived<ClickableWidget>(current_widget_).get();
}

void GripTracker::Reset() {
    current_data_.info        = Grippable::GripInfo();
    current_data_.position    = Point3f::Zero();
    current_data_.orientation = Rotationf::Identity();
    activation_data_ = current_data_;
    current_widget_.reset();
}

bool GripTracker::UpdateCurrentData_(const Event &event, WidgetPtr &widget) {
    // Get the grip data, including the full GripInfo.
    Data_ data;
    if (! GetGripData_(event, true, data))
        return false;

    widget = data.info.widget;
    if (widget && ! widget->IsInteractionEnabled())
        widget.reset();
    current_data_ = data;

    // Update the Controller grip hover. The target point needs to be converted
    // from world coordinates into the Controller's object coordinates.
     if (widget && ! grippable_path_.empty()) {
        const Point3f pt = ToControllerCoords(data.info.target_point);
        GetController()->ShowGripHover(true, pt, data.info.color);
    }
    else {
        GetController()->ShowGripHover(false, Point3f::Zero(), data.info.color);
    }
    return true;
}

bool GripTracker::GetGripData_(const Event &event, bool add_info,
                               Data_ &data) const {
    if (event.flags.Has(Event::Flag::kPosition3D) &&
        event.flags.Has(Event::Flag::kOrientation) &&
        event.device == GetDevice()) {

        data.position    = event.position3D;
        data.orientation = event.orientation;

        if (add_info) {
            Grippable::GripInfo &info = data.info;
            info = Grippable::GripInfo();
            info.event      = event;
            info.controller = GetController();
            info.guide_direction =
                event.orientation * info.controller->GetGuideDirection();

            // Let the Grippable (if any) fill in the rest of the GripInfo.
            if (grippable_) {
                grippable_->UpdateGripInfo(info);

                // Update the guide in the Controller.
                info.controller->SetGripGuideType(info.guide_type);
            }
        }

        return true;
    }
    return false;
}

void GripTracker::UpdateControllers_(bool is_active) {
    GetController()->SetTriggerMode(Trigger::kGrip, is_active);
    GetOtherController()->ShowAll(! is_active);
    if (grippable_)
        grippable_->ActivateGrip(GetController()->GetHand(), is_active);
}
