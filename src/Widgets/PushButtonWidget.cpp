#include "PushButtonWidget.h"

void PushButtonWidget::AddFields() {
    AddField(is_toggle_);
    ClickableWidget::AddFields();
}

void PushButtonWidget::Click(const ClickInfo &info) {
    ASSERT(IsInteractionEnabled());
    if (IsToggle())
        toggle_state_ = ! toggle_state_;
    ClickableWidget::Click(info);
}
