#include "Panes/ButtonPane.h"

#include "ClickInfo.h"
#include "SG/Search.h"
#include "Widgets/PushButtonWidget.h"

void ButtonPane::AddFields() {
    AddField(is_toggle_);
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

ClickableWidgetPtr ButtonPane::GetActivationWidget() const {
    return button_;
}

bool ButtonPane::CanFocus() const {
    return GetButton().IsInteractionEnabled();
}

void ButtonPane::SetFocus(bool is_focused) {
    // Nothing special to do when focus changes.
}

void ButtonPane::Activate() {
    // Activation is equivalent to clicking the button.
    auto &but = GetButton();
    ASSERT(but.IsInteractionEnabled());
    ClickInfo info;
    info.widget = &but;
    but.Click(info);
}

void ButtonPane::CopyContentsFrom(const Parser::Object &from, bool is_deep) {
    BoxPane::CopyContentsFrom(from, is_deep);

    // Panes will be added as children to the PushButtonWidget when
    // CreationDone() is called for this instance. Remove any created by the
    // cloning.
    GetButton().ClearChildren();
}
