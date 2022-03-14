#include "Panes/RadioButtonPane.h"

#include "Event.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Widgets/PushButtonWidget.h"

void RadioButtonPane::AddFields() {
    AddField(state_);
    Pane::AddFields();
}

void RadioButtonPane::SetState(bool new_state) {
    if (group_) {
        if (new_state && ! state_) {
            state_ = true;
            UpdateState_();
            for (auto &weak_but: group_->buttons) {
                RadioButtonPanePtr but(weak_but);
                if (but && but.get() != this && but->GetState()) {
                    but->state_ = false;
                    but->UpdateState_();
                }
            }
        }
    }
    else {
        state_ = new_state;
        UpdateState_();
    }
}

void RadioButtonPane::CreateGroup(
    const std::vector<RadioButtonPanePtr> &buttons, size_t selected_index) {
    // Create the Group_ with weak pointers to the buttons to prevent reference
    // cycles; deleting the RadioButtonPane instances should work properly.
    std::shared_ptr<Group_> group(new Group_);
    for (auto &but: buttons) {
        ASSERT(but);
        group->buttons.push_back(std::weak_ptr<RadioButtonPane>(but));
    }

    // Store the Group_ pointer in all the buttons. No button should already
    // belong to a group.
    for (auto &but: buttons) {
        ASSERT(! but->group_);
        but->group_ = group;
    }

    // Turn on the selected button.
    ASSERT(selected_index < buttons.size());
    buttons[selected_index]->Activate();
}

void RadioButtonPane::PostSetUpIon() {
    Pane::PostSetUpIon();

    // Set up the PushButtonWidget.
    auto button = SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, "Button");
    button->GetClicked().AddObserver(
        this, [this](const ClickInfo &){ Toggle_(); });
}

void RadioButtonPane::Activate() {
    Toggle_();
}

void RadioButtonPane::Deactivate() {
    // Nothing to do here.
}

bool RadioButtonPane::HandleEvent(const Event &event) {
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

void RadioButtonPane::Toggle_() {
    if (group_) {
        // If this button is part of a group, toggle it only if it is off.
        if (! state_.GetValue())
            SetState(true);
    }
    else {
        // Otherwise, just toggle.
        SetState(! state_.GetValue());
    }
}

void RadioButtonPane::UpdateState_() {
    SG::FindNodeUnderNode(*this, "Check")->SetEnabled(state_.GetValue());
}
