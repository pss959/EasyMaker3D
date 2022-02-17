#include "Panes/DropdownPane.h"

#include "Event.h"
#include "Widgets/PushButtonWidget.h"

void DropdownPane::AddFields() {
    AddField(choices_);
    AddField(choice_index_);
    BoxPane::AddFields();
}

bool DropdownPane::IsValid(std::string &details) {
    if (! BoxPane::IsValid(details))
        return false;
    const auto &choices = choices_.GetValue();
    if (choice_index_ < 0) {
        if (! choices.empty()) {
            details = "No initial choice.";
            return false;
        }
    }
    else {
        if (static_cast<size_t>(choice_index_) >= choices.size()) {
            details = "Choice index out of range.";
            return false;
        }
    }
    return true;
}

void DropdownPane::CreationDone() {
    BoxPane::CreationDone();

    if (! IsTemplate()) {
        if (choice_index_ >= 0)
            choice_ = choices_.GetValue()[choice_index_];
        else
            choice_ = ".";  // Cannot be empty.

        text_pane_          = FindTypedPane<TextPane>("CurrentChoiceText");
        choice_pane_        = FindTypedPane<ScrollingPane>("ChoicePane");
        choice_button_pane_ = FindTypedPane<ButtonPane>("ChoiceButton");

        // Set up the button to activate.
        auto but = FindTypedPane<ButtonPane>("ButtonPane");
        but->GetButton().GetClicked().AddObserver(
            this, [&](const ClickInfo &){ Activate(); });

        // Remove the ScrollingPane and FileButton from the list of Panes so
        // they do not show up by default.
        RemovePane(choice_pane_);
        RemovePane(choice_button_pane_);

        // Show the correct initial text.
        text_pane_->SetText(choice_);

        need_to_update_choice_pane_ = choices_.WasSet();
    }
}

void DropdownPane::SetChoices(const std::vector<std::string> &choices,
                              size_t index) {
    choices_ = choices;
    if (choices.empty()) {
        choice_index_ = -1;
        choice_ = ".";  // Cannot be empty.
    }
    else {
        ASSERT(index < choices.size());
        choice_index_ = index;
        choice_ = choices[index];
    }
    text_pane_->SetText(choice_);
    need_to_update_choice_pane_ = true;
}

void DropdownPane::Activate() {
    // Update choice buttons if necessary.
    UpdateChoicePane_();

    // XXXX Need to use smaller of ScrollingPane MinSize and computed MinSize
    // for all buttons.

    // Set the size and relative position of the ScrollingPane. Offset the Pane
    // forward a little.
    const Vector2f size = choice_pane_->GetBaseSize();
    choice_pane_->SetSizeWithinContainer(
        size, ComputeSubPaneRect(GetSize(), size, Point2f(0, 2)), true);

    // Show it.
    choice_pane_->SetEnabled(true);
}

void DropdownPane::Deactivate() {
    choice_pane_->SetEnabled(false);
}

bool DropdownPane::HandleEvent(const Event &event) {
    // XXXX Allow changing selection.
    return false;
}

void DropdownPane::UpdateChoicePane_() {
    if (! need_to_update_choice_pane_ || ! choice_pane_)
        return;

    // Clone the choice ButtonPane for each choice.
    std::vector<PanePtr> buttons;
    for (const auto &choice: choices_.GetValue()) {
        auto but = choice_button_pane_->CloneTyped<ButtonPane>(true);
        auto text = but->FindTypedPane<TextPane>("ButtonText");
        text->SetText(choice);
        but->GetButton().GetClicked().AddObserver(
            this, [&](const ClickInfo &){ ChoiceButtonClicked_(choice); });
        but->SetEnabled(true);
        buttons.push_back(but);
    }
    ASSERT(choice_pane_->GetContentsPane());
    choice_pane_->GetContentsPane()->ReplacePanes(buttons);

    // Set the minimum width of the choice Pane based on the text pane sizes.
    // This has to be done after the Panes are added to the choice Pane so that
    // the Ion text is set up.
    float min_width = 0;
    for (const auto &pane: choice_pane_->GetContentsPane()->GetPanes())
        min_width = std::max(min_width, pane->GetBaseSize()[0]);
    choice_pane_->SetMinSize(
        Vector2f(min_width + 4, choice_pane_->GetMinSize()[1]));

    need_to_update_choice_pane_ = false;
}

void DropdownPane::ChoiceButtonClicked_(const std::string &text) {
    std::cerr << "XXXX CLICKED ON CHOICE '" << text << "'\n";
    text_pane_->SetText(text);
    Deactivate();
}
