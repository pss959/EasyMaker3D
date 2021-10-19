#include "Widgets/TargetWidgetBase.h"

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
#if XXXX
    // If there was an ITargetable hit during the last position of the current
    // drag AND isAlternateMode is true, ask it if the ray still hits it.
    hit = new RaycastHit();
    ITargetable it = null;
    if (_prevTargetable != null && isAlternateMode) {
        RaycastHit? altHit = _prevTargetable.GetAlternateHit(ray);
        if (altHit != null) {
            hit = altHit.Value;
            it  = _prevTargetable;
        }
    }
    // Otherwise, intersect with the scene.
    if (it == null && Physics.Raycast(ray, out hit))
        it = UT.FindComponentAbove<ITargetable>(hit.transform.gameObject);

    _prevTargetable = it;
    return it;
#endif
    return WidgetPtr();  // XXXX
}
