#include "Widgets/GenericWidget.h"

void GenericWidget::StartDrag(const DragInfo &info) {
    DraggableWidget::StartDrag(info);
    SetActive(true);
}

void GenericWidget::EndDrag() {
    DraggableWidget::EndDrag();
    SetActive(false);
}
