#include "Widgets/GenericWidget.h"

void GenericWidget::StartDrag(const DragInfo &info) {
    DraggableWidget::StartDrag(info);
    SetActive(true);
    dragged_.Notify(&info, true);
}

void GenericWidget::ContinueDrag(const DragInfo &info) {
    DraggableWidget::ContinueDrag(info);
    dragged_.Notify(&info, false);
}

void GenericWidget::EndDrag() {
    DraggableWidget::EndDrag();
    dragged_.Notify(nullptr, false);
    SetActive(false);
}

void GenericWidget::UpdateHoverPoint(const Point3f &point) {
    hovered_.Notify(point);
}
