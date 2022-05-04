#include "Panes/CheckboxPane.h"

#include "Event.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Widgets/PushButtonWidget.h"

void CheckboxPane::AddFields() {
    AddField(state_);
    Pane::AddFields();
}

void CheckboxPane::SetState(bool new_state) {
    state_ = new_state;
    UpdateState_();
}

void CheckboxPane::PostSetUpIon() {
    Pane::PostSetUpIon();

    // Set up the PushButtonWidget.
    button_ = SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, "Button");
    button_->GetClicked().AddObserver(
        this, [this](const ClickInfo &){ Toggle_(); });
}

ClickableWidgetPtr CheckboxPane::GetActivationWidget() const {
    return button_;
}

bool CheckboxPane::CanFocus() const {
    return true;
}

void CheckboxPane::SetFocus(bool is_focused) {
    // Nothing special to do when focus changes.
}

void CheckboxPane::Activate() {
    // Activation is equivalent to clicking the checkbox.
    Toggle_();
}

void CheckboxPane::Toggle_() {
    SetState(! state_.GetValue());
}

void CheckboxPane::UpdateState_() {
    SG::FindNodeUnderNode(*this, "Check")->SetEnabled(state_.GetValue());
}
