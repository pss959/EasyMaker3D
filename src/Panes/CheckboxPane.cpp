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
    auto button = SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, "Button");
    button->GetClicked().AddObserver(
        this, [this](const ClickInfo &){ Toggle_(); });
}

void CheckboxPane::Activate() {
    Toggle_();
}

void CheckboxPane::Deactivate() {
    // Nothing to do here.
}

bool CheckboxPane::HandleEvent(const Event &event) {
    // Space bar or Enter key toggles state.
    if (event.flags.Has(Event::Flag::kKeyPress)) {
        const std::string key_string = event.GetKeyString();
        if (key_string == " " || key_string == "Enter") {
            Toggle_();
            return true;
        }
    }
    return false;
}

void CheckboxPane::Toggle_() {
    SetState(! state_.GetValue());
}

void CheckboxPane::UpdateState_() {
    SG::FindNodeUnderNode(*this, "Check")->SetEnabled(state_.GetValue());
}
