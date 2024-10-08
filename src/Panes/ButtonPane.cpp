//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panes/ButtonPane.h"

#include "Place/ClickInfo.h"
#include "SG/Search.h"
#include "Widgets/PushButtonWidget.h"

void ButtonPane::AddFields() {
    AddField(is_toggle_.Init("is_toggle", false));

    BoxPane::AddFields();
}

void ButtonPane::CreationDone() {
    BoxPane::CreationDone();

    if (! IsTemplate()) {
        // Add all Panes as children of the PushButtonWidget.
        auto &but = GetButton();
        but.SetIsToggle(is_toggle_);
        for (auto &pane: GetPanes())
            but.AddChild(pane);
    }
}

SG::Node & ButtonPane::GetAuxParent() {
    return GetButton();
}

PushButtonWidget & ButtonPane::GetButton() const {
    if (! button_)
        button_ = SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, "Button");
    return *button_;
}

void ButtonPane::SetInteractionEnabled(bool enabled) {
    GetButton().SetInteractionEnabled(enabled);
}

bool ButtonPane::IsInteractionEnabled() const {
    return GetButton().IsInteractionEnabled();
}

ClickableWidgetPtr ButtonPane::GetActivationWidget() const {
    return button_;
}

bool ButtonPane::CanFocus(FocusReason reason) const {
    // Don't focus on this ButtonPane when activated if the flag is false.
    return GetButton().IsInteractionEnabled() &&
        (reason != FocusReason::kActivation || should_focus_on_activation_);
}

BorderPtr ButtonPane::GetFocusBorder() const {
    return GetButton().IsInteractionEnabled() ? GetBorder() : BorderPtr();
}

void ButtonPane::CopyContentsFrom(const Parser::Object &from, bool is_deep) {
    BoxPane::CopyContentsFrom(from, is_deep);

    // Panes will be added as children to the PushButtonWidget when
    // CreationDone() is called for this instance. Remove any created by the
    // cloning.
    GetButton().ClearChildren();
}
