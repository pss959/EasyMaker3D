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
    std::cerr << "XXXX Drag on " << info.path.ToString() << "\n";


#if XXXX
    SG::Hit hit;
    if (IntersectRay_(info.ray, info.is_alternate_mode, hit)) {
        // Store the position and direction in stage coordinates for use by
        // Tools.
        _target.position  = position;
        _target.direction = direction;

        // Move the target geometry.
        Move();
        Motion.Invoke(this);
    }
#endif
}

void PointTargetWidget::EndDrag() {
    // XXXX
}

void PointTargetWidget::ShowExtraSnapFeedback(bool is_snapping) {
    // XXXX
}
