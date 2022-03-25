#include "Widgets/NewPointWidget.h"

#include "Util/Assert.h"
#include "Widgets/Slider2DWidget.h"

void NewPointWidget::StartDrag(const DragInfo &info) {
    ASSERT(delegate_widget_);
    // Hide this Widget and start the drag on the delegate widget.
    SetEnabled(false);
    delegate_widget_->StartDrag(info);
}

void NewPointWidget::ContinueDrag(const DragInfo &info) {
    ASSERT(delegate_widget_);
    delegate_widget_->ContinueDrag(info);
}

void NewPointWidget::EndDrag() {
    ASSERT(delegate_widget_);
    delegate_widget_->EndDrag();
}
