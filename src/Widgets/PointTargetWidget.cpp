#include "Widgets/PointTargetWidget.h"

#include "Util/Assert.h"

void PointTargetWidget::StartDrag(const DragInfo &info) {
    // XXXX
    DraggableWidget::StartDrag(info);
    ASSERTM(! info.is_grip, "PointTargetWidget does not do grip drags");

    // Turn off intersections during the drag.
    SetEnabled(Flag::kIntersectAll, false);

    SetActive(true);
}

void PointTargetWidget::ContinueDrag(const DragInfo &info) {
    // If there is a Widget on the path that can receive a target, ask it where
    // to place the target.
    if (auto widget = GetReceiver(info)) {
        Point3f        position;
        Vector3f       direction;
        Dimensionality snapped_dims;
        widget->PlacePointTarget(info, position, direction, snapped_dims);

        // Move the PointTargetWidget.
        SetTranslation(Vector3f(position));
        SetRotation(Rotationf::RotateInto(Vector3f::AxisY(), direction));
        // XXXX Notify?
    }
}

void PointTargetWidget::EndDrag() {
    // XXXX
    SetEnabled(Flag::kIntersectAll, true);
}

void PointTargetWidget::ShowExtraSnapFeedback(bool is_snapping) {
    // XXXX
}
