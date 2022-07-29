#include "Panes/DropdownPane.h"

#include <algorithm>

#include "Base/Event.h"
#include "Panes/ButtonPane.h"
#include "Panes/ScrollingPane.h"
#include "Util/General.h"
#include "Widgets/PushButtonWidget.h"

void DropdownPane::AddFields() {
    AddField(choices_.Init("choices"));
    AddField(choice_index_.Init("choice_index", -1));

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

        text_pane_              = FindTypedPane<TextPane>("CurrentChoiceText");
        menu_pane_              = FindTypedPane<ScrollingPane>("MenuPane");
        menu_button_pane_       = FindTypedPane<ButtonPane>("MenuButton");
        activation_button_pane_ = FindTypedPane<ButtonPane>("ButtonPane");

        // Get the width of the ButtonPane that shows the current text when it
        // has no text and also the width of the scroll bar. The larger of
        // these two will be added to the width of the longest choice menu
        // string to get the real base width.
        menu_extra_width_ = std::max(activation_button_pane_->GetBaseSize()[0],
                                     menu_pane_->GetScrollBarWidth());

        // Remove the ScrollingPane and FileButton from the list of Panes so
        // they do not show up by default.
        RemovePane(menu_pane_);
        RemovePane(menu_button_pane_);

        // Show the correct initial text.
        text_pane_->SetText(choice_);
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

    UpdateMenuPane_();
}

void DropdownPane::SetChoice(size_t index) {
    ASSERT(index < choices_.GetValue().size());

    if (choice_index_ < 0 || index != static_cast<size_t>(choice_index_)) {
        // Turn off old button if any.
        if (choice_index_ >= 0)
            menu_button_panes_[choice_index_]->GetButton().SetActive(false);

        choice_index_ = index;
        choice_       = choices_.GetValue()[index];
        text_pane_->SetText(choice_);
    }

    // Make sure the current button is active.
    menu_button_panes_[choice_index_]->GetButton().SetActive(true);
}

void DropdownPane::SetChoiceFromString(const std::string &choice) {
    const auto &choices = choices_.GetValue();
    const auto it = std::find(choices.begin(), choices.end(), choice);
    ASSERTM(it != choices.end(), "No such choice: " + choice);
    SetChoice(std::distance(choices.begin(), it));
}

const ScrollingPane & DropdownPane::GetMenuPane() const {
    ASSERT(menu_pane_);
    return *menu_pane_;
}

void DropdownPane::PostSetUpIon() {
    // Now that all of the TextPanes should have the correct size, update the
    // base size.
    if (choices_.WasSet())
        UpdateMenuPane_();
}

ClickableWidgetPtr DropdownPane::GetActivationWidget() const {
    return activation_button_pane_->GetActivationWidget();
}

bool DropdownPane::CanFocus() const {
    return true;
}

void DropdownPane::Activate() {
    // Set the size and relative position of the ScrollingPane. Offset the Pane
    // forward a little.
    const Vector2f size = menu_pane_->GetBaseSize();
    menu_pane_->SetLayoutSize(size);
    PositionSubPane(*menu_pane_, Point2f(0, 2), true);

    // Show it.
    menu_pane_->SetEnabled(true);
    ASSERT(IsActive());
}

void DropdownPane::Deactivate() {
    // Hide the choice Pane.
    menu_pane_->SetEnabled(false);
}

bool DropdownPane::IsActive() const {
    return menu_pane_->IsEnabled();
}

bool DropdownPane::HandleEvent(const Event &event) {
    bool handled = false;
    // Handle events only if active (choice Pane is visible).
    if (IsActive() && event.flags.Has(Event::Flag::kKeyPress)) {
        const std::string key_string = event.GetKeyString();

        // Up/down keys change selected choice.
        if (key_string == "Up") {
            if (choice_index_ > 0)
                SetChoice(choice_index_ - 1);
            handled = true;
        }
        else if (key_string == "Down") {
            if (static_cast<size_t>(choice_index_ + 1) <
                choices_.GetValue().size())
                SetChoice(choice_index_ + 1);
            handled = true;
        }

        // Enter or space selects current choice.
        else if (key_string == "Enter" || key_string == " ") {
            ChoiceButtonClicked_(choice_index_);
            handled = true;
        }

        // Escape just closes the choice Pane.
        else if (key_string == "Escape") {
            menu_pane_->SetEnabled(false);
            handled = true;
        }
    }
    return handled;
}

void DropdownPane::AddEnabledWidgets(std::vector<WidgetPtr> &widgets) const {
    // If the menu is visible, add all of its enabled buttons.
    if (IsActive()) {
        for (auto &pane: menu_pane_->GetContentsPane()->GetPanes()) {
            ASSERT(pane->GetInteractor());
            pane->GetInteractor()->AddEnabledWidgets(widgets);
        }
    }

    // Otherwise, just add the button that opens the choice menu.
    else {
        auto but = GetActivationWidget();
        if (but->IsInteractionEnabled())
            widgets.push_back(but);
    }
}

Vector2f DropdownPane::ComputeBaseSize() const {
    return Vector2f(menu_pane_->GetMinSize()[0],
                    BoxPane::ComputeBaseSize()[1]);
}

void DropdownPane::UpdateMenuPane_() {
    ASSERT(menu_pane_);

    // Clone the menu ButtonPane for each choice.
    const auto &choices = choices_.GetValue();
    for (size_t i = 0; i < choices.size(); ++i) {
        const std::string &choice = choices[i];
        auto but = menu_button_pane_->CloneTyped<ButtonPane>(true);
        auto text = but->FindTypedPane<TextPane>("ButtonText");
        text->SetText(choice);
        but->GetButton().GetClicked().AddObserver(
            this, [&, i](const ClickInfo &){ ChoiceButtonClicked_(i); });
        but->SetEnabled(true);
        menu_button_panes_.push_back(but);
    }
    ASSERT(menu_pane_->GetContentsPane());
    menu_pane_->GetContentsPane()->ReplacePanes(
        Util::ConvertVector<PanePtr, ButtonPanePtr>(
            menu_button_panes_, [](const ButtonPanePtr &p){ return p; }));

    // Set the minimum width of the menu Pane based on the text pane sizes.
    // This has to be done after the Panes are added to the menu Pane so that
    // the Ion text is set up.
    const auto &panes = menu_pane_->GetContentsPane()->GetPanes();
    ASSERT(! panes.empty());
    Vector2f area_size(0, 0);
    for (const auto &pane: panes)
        area_size[0] = std::max(area_size[0], pane->GetBaseSize()[0]);
    area_size[0] += menu_pane_->GetScrollBarWidth();

    // Compute the height of the menu Pane based on the number of choices and
    // text height. Clamp to a reasonable maximum.
    area_size[1] = std::min(200.f, panes.size() * panes[0]->GetBaseSize()[1]);
    menu_pane_->SetScrollAreaSize(area_size);

    // Set the layout size of the menu Pane to its base size.
    menu_pane_->SetLayoutSize(menu_pane_->GetBaseSize());

    BaseSizeChanged();
}

void DropdownPane::ChoiceButtonClicked_(size_t index) {
    choice_index_ = index;
    choice_       = choices_.GetValue()[index];
    text_pane_->SetText(choice_);

    // Hide the choice pane.
    menu_pane_->SetEnabled(false);

    choice_changed_.Notify(choice_);
}
