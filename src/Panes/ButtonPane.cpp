#include "Panes/ButtonPane.h"

#include "ClickInfo.h"
#include "SG/Search.h"

PushButtonWidget & ButtonPane::GetButton() const {
    if (! button_)
        button_ = SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, "Button");
    return *button_;
}

void ButtonPane::SetInteractionEnabled(bool enabled) {
    GetButton().SetInteractionEnabled(enabled);
}

void ButtonPane::PreSetUpIon() {
    BoxPane::PreSetUpIon();

    // Add all Panes as children of the PushButtonWidget.
    auto &but = GetButton();
    for (auto &pane: GetPanes())
        but.AddChild(pane);
}

bool ButtonPane::IsInteractionEnabled() const {
    return GetButton().IsInteractionEnabled();
}

void ButtonPane::Activate() {
    auto &but = GetButton();
    ASSERT(but.IsInteractionEnabled());
    ClickInfo info;
    info.widget = &but;
    but.Click(info);
}
