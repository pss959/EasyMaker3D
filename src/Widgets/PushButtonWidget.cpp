//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Widgets/PushButtonWidget.h"

void PushButtonWidget::AddFields() {
    AddField(is_toggle_.Init("is_toggle", false));

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
