#include "Panes/DropdownPane.h"

#include <algorithm>

#include "Event.h"
#include "Panes/ButtonPane.h"
#include "Panes/ScrollingPane.h"
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

        if (choices_.WasSet())
            UpdateChoicePane_();
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

    UpdateChoicePane_();
}

void DropdownPane::SetChoice(size_t index) {
    ASSERT(index < choices_.GetValue().size());
    choice_index_ = index;
    text_pane_->SetText(choices_.GetValue()[index]);
}

Pane & DropdownPane::GetChoicePane() const {
    ASSERT(choice_pane_);
    return *choice_pane_;
}

void DropdownPane::Activate() {
    // Set the size and relative position of the ScrollingPane. Offset the Pane
    // forward a little.
    const Vector2f size = choice_pane_->GetBaseSize();
    choice_pane_->SetLayoutSize(size);
    PositionSubPane(*choice_pane_, Point2f(0, 2), true);

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

Vector2f DropdownPane::ComputeBaseSize() const {
    const Vector2f button_size = choice_pane_->GetContentsPane()->GetBaseSize();
    const Vector2f min_scroll_size = choice_pane_->GetMinSize();

    // For width, use the larger of the ScrollingPane MinSize and the computed
    // MinSize for all buttons.  For height, use the smaller of ScrollingPane
    // MinSize and the computed MinSize for all buttons.
    return Vector2f(std::max(min_scroll_size[0], button_size[0]),
                    std::min(min_scroll_size[1], button_size[1]));
}

void DropdownPane::UpdateChoicePane_() {
    ASSERT(choice_pane_);
    // std::cerr << "XXXX UCP for " << GetDesc() << "\n";

    // Clone the choice ButtonPane for each choice.
    std::vector<PanePtr> buttons;
    const auto &choices = choices_.GetValue();
    for (size_t i = 0; i < choices.size(); ++i) {
        const std::string &choice = choices[i];
        auto but = choice_button_pane_->CloneTyped<ButtonPane>(true);
        auto text = but->FindTypedPane<TextPane>("ButtonText");
        text->SetText(choice);
        but->GetButton().GetClicked().AddObserver(
            this, [&, i](const ClickInfo &){ ChoiceButtonClicked_(i); });
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
    choice_pane_->SetScrollAreaSize(
        Vector2f(min_width + 4, choice_pane_->GetMinSize()[1]));
    //std::cerr << "XXXX    CP base size = "
    //          << choice_pane_->GetBaseSize() << "\n";

    // Set the layout size of the choice Pane to its base size.
    choice_pane_->SetLayoutSize(choice_pane_->GetBaseSize());

    BaseSizeChanged();
}

void DropdownPane::ChoiceButtonClicked_(size_t index) {
    choice_index_ = index;
    text_pane_->SetText(choices_.GetValue()[index]);
    Deactivate();
}
