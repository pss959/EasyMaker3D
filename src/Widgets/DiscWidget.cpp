#include "DiscWidget.h"

void DiscWidget::StartDrag(const DragInfo &info) {
    std::cerr << "XXXX DW " << GetName() << " StartDrag\n";
}

void DiscWidget::Drag(const DragInfo &info) {
    std::cerr << "XXXX DW " << GetName() << " Drag\n";
}

void DiscWidget::EndDrag() {
    std::cerr << "XXXX DW " << GetName() << " EndDrag\n";
}

