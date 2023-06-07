#include "Widgets/TargetWidgetBase.h"

#include "SG/ColorMap.h"

void TargetWidgetBase::CreationDone() {
    DraggableWidget::CreationDone();
    SetColorNamePrefix("Target");
}

void TargetWidgetBase::StartDrag(const DragInfo &info) {
    DraggableWidget::StartDrag(info);
    ASSERTM(info.trigger == Trigger::kPointer,
            GetTypeName() + " handles only pointer drags");

    // Turn off intersections during the drag.
    SetFlagEnabled(Flag::kIntersectAll, false);

    SetActive(true);

    // Let the derived class update anything it needs to.
    StartTargetPlacement();
}

void TargetWidgetBase::ContinueDrag(const DragInfo &info) {
    DraggableWidget::ContinueDrag(info);

    // If there is a Widget on the path that can receive a target, let the
    // derived class tell it how to place the target.
    if (auto widget = GetReceiver_(info)) {
        PlaceTarget(*widget, info);
        NotifyChanged();
    }
}

void TargetWidgetBase::EndDrag() {
    DraggableWidget::EndDrag();

    // Let the derived class update anything it needs to.
    EndTargetPlacement();

    SetActive(false);
    SetFlagEnabled(Flag::kIntersectAll, true);
}

void TargetWidgetBase::ShowSnapFeedback(bool is_snapping) {
    if (is_snapping == snap_feedback_active_)
        return;

    // Change the state of the Widget, but do not invoke the callbacks as it is
    // not being dragged.
    SetActive(is_snapping, false);

    // Let the derived class do anything extra it needs to do.
    ShowExtraSnapFeedback(is_snapping);

    snap_feedback_active_ = is_snapping;
}

Color TargetWidgetBase::GetActiveColor() {
    return SG::ColorMap::SGetColor("TargetActiveColor");
}

WidgetPtr TargetWidgetBase::GetReceiver_(const DragInfo &info) {
    // Look upwards in the Hit path for a Widget that can receive a target.
    auto can_receive = [](const Node &n){
        auto widget = dynamic_cast<const Widget *>(&n);
        return widget && widget->CanReceiveTarget();
    };
    return Util::CastToDerived<Widget>(
        info.hit.path.FindNodeUpwards(can_receive));
}
