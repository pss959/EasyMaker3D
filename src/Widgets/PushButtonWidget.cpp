#include "Widgets/PushButtonWidget.h"

void PushButtonWidget::AddFields() {
    AddField(is_toggle_);
    ClickableWidget::AddFields();
}

void PushButtonWidget::SetToggleState(bool state) {
    const bool new_state = IsToggle() && state;
    if (toggle_state_ != new_state) {
        toggle_state_ = new_state;
        SetActive(toggle_state_);
    }
}

void PushButtonWidget::Click(const ClickInfo &info) {
    ASSERT(IsInteractionEnabled());
    if (IsToggle())
        SetToggleState(! toggle_state_);
    ClickableWidget::Click(info);
}
