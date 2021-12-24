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
    auto can_receive = [](const Node &n){
        auto widget = dynamic_cast<const Widget *>(&n);
        return widget && widget->CanReceiveTarget();
    };

    auto widget = Util::CastToDerived<Widget>(
        info.path_to_intersected_node.FindNodeUpwards(can_receive));
    if (widget) {
#if XXXX
        Point3f  position;
        Vector3f direction;
        Dimensionality snapped_dims;
        widget->PlacePointTarget(info.hit, info.is_alternate_mode,
                                 GetStagePath(), position, direction,
                                 snapped_dims);
        // Store the position and direction in stage coordinates for use by
        // Tools.
        _target.position  = position;
        _target.direction = direction;

        // Move the target geometry.
        Move();
        Motion.Invoke(this);
#endif
    }
}

void PointTargetWidget::EndDrag() {
    // XXXX
    SetEnabled(Flag::kIntersectAll, true);
}

void PointTargetWidget::ShowExtraSnapFeedback(bool is_snapping) {
    // XXXX
}
