#include "Panes/CheckboxPane.h"

#include "Base/Event.h"
#include "Base/Tuning.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Widgets/PushButtonWidget.h"

void CheckboxPane::AddFields() {
    AddField(state_.Init("state", false));

    Pane::AddFields();
}

void CheckboxPane::SetState(bool new_state) {
    state_ = new_state;
    UpdateState_();
}

void CheckboxPane::CreationDone() {
    LeafPane::CreationDone();

    if (! IsTemplate()) {
        // Offset in Z for button and checkmark.
        const Vector3f z_offset(0, 0, TK::kPaneZOffset);

        // Set up the check geometry.
        SG::FindNodeUnderNode(*this, "Check")->SetTranslation(z_offset);

        // Set up the PushButtonWidget.
        button_ = SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, "Button");
        button_->SetTranslation(2 * z_offset);
        button_->GetClicked().AddObserver(
            this, [this](const ClickInfo &){ Toggle_(); });
    }
}

ClickableWidgetPtr CheckboxPane::GetActivationWidget() const {
    return button_;
}

BorderPtr CheckboxPane::GetFocusBorder() const {
    return GetBorder();
}

void CheckboxPane::Toggle_() {
    SetState(! state_.GetValue());
    state_changed_.Notify();
}

void CheckboxPane::UpdateState_() {
    SG::FindNodeUnderNode(*this, "Check")->SetEnabled(state_.GetValue());
}
