//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panes/RadioButtonPane.h"

#include "Base/Event.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"
#include "Widgets/PushButtonWidget.h"

void RadioButtonPane::AddFields() {
    AddField(state_.Init("state", false));

    LeafPane::AddFields();
}

void RadioButtonPane::CreationDone() {
    LeafPane::CreationDone();

    if (! IsTemplate()) {
        // Offset in Z for button and checkmark.
        const Vector3f z_offset(0, 0, TK::kPaneZOffset);

        button_ = SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, "Button");
        check_  = SG::FindNodeUnderNode(*this, "Check");

        check_->SetTranslation(z_offset);
        button_->SetTranslation(2 * z_offset);
    }
}

void RadioButtonPane::SetState(bool new_state) {
    bool changed = false;
    if (group_) {
        if (new_state && ! state_) {
            state_ = true;
            UpdateState_();
            changed = true;
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
        if (state_ != new_state) {
            state_ = new_state;
            UpdateState_();
            changed = true;
        }
    }
    if (changed)
        state_changed_.Notify(index_in_group_);
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
    size_t index = 0;
    for (auto &but: buttons) {
        ASSERT(! but->group_);
        but->group_ = group;
        but->index_in_group_ = index++;
    }

    // Turn on the selected button.
    ASSERT(selected_index < buttons.size());
    buttons[selected_index]->Activate();
}

ClickableWidgetPtr RadioButtonPane::GetActivationWidget() const {
    return button_;
}

BorderPtr RadioButtonPane::GetFocusBorder() const {
    return GetBorder();
}

void RadioButtonPane::Activate() {
    // Activation is equivalent to clicking the radio button.
    Toggle_();
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
    check_->SetEnabled(state_.GetValue());
}
