#include "Widgets/TargetWidgetBase.h"

#include "Managers/ColorManager.h"

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

WidgetPtr TargetWidgetBase::GetReceiver(const DragInfo &info) {
    // Look upwards in the Hit path for a Widget that can receive a target.
    auto can_receive = [](const Node &n){
        auto widget = dynamic_cast<const Widget *>(&n);
        return widget && widget->CanReceiveTarget();
    };
    return Util::CastToDerived<Widget>(
        info.hit.path.FindNodeUpwards(can_receive));
}

Color TargetWidgetBase::GetActiveColor() {
    return ColorManager::GetSpecialColor("TargetActiveColor");
}
