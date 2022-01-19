#include "Panes/DropdownPane.h"

#include "Event.h"

void DropdownPane::AddFields() {
    AddField(choices_);
    AddField(choice_index_);
    BoxPane::AddFields();
}

bool DropdownPane::IsValid(std::string &details) {
    if (! BoxPane::IsValid(details))
        return false;
    if (choice_index_ >= 0 &&
        static_cast<size_t>(choice_index_) >= choices_.GetValue().size()) {
        details = "Choice index out of range.";
        return false;
    }
    return true;
}

void DropdownPane::SetChoices(const std::vector<std::string> &choices) {
    choices_ = choices;
    if (choices.empty()) {
        choice_index_ = -1;
        choice_.clear();
    }
    else {
        choice_index_ = 0;
        choice_ = choices[0];
    }
}

void DropdownPane::PostSetUpIon() {
    BoxPane::PostSetUpIon();

    /* XXXX
    // Set up the PushButtonWidget.
    auto button = SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, "Button");
    button->GetClicked().AddObserver(
        this, [&](const ClickInfo &){ Activate(); });
    */
}

void DropdownPane::Activate() {
    std::cerr << "XXXX Activating " << GetDesc() << "\n";
    // XXXX
}

void DropdownPane::Deactivate() {
    std::cerr << "XXXX Deactivating " << GetDesc() << "\n";
    // XXXX
}

bool DropdownPane::HandleEvent(const Event &event) {
    // XXXX Allow changing selection.
    return false;
}
