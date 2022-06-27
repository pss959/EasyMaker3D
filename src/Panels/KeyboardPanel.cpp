#include "Panels/KeyboardPanel.h"

#include <vector>

#include "Base/VirtualKeyboard.h"
#include "Panes/ButtonPane.h"
#include "Panes/ContainerPane.h"
#include "Panes/TextInputPane.h"
#include "Panes/TextPane.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Widgets/PushButtonWidget.h"

KeyboardPanel::KeyboardPanel() : virtual_keyboard_(new VirtualKeyboard) {
}

void KeyboardPanel::InitInterface() {
    // Create a Key_ instance for each ButtonPane and set up a click callback.
    std::vector<ButtonPanePtr> button_panes;
    FindButtonPanes_(GetPane(), button_panes);
    keys_.reserve(button_panes.size());
    for (const ButtonPanePtr &bp: button_panes) {
        ASSERT(bp->GetPanes().size() == 1U);
        auto tp = Util::CastToDerived<TextPane>(bp->GetPanes()[0]);
        ASSERT(tp);

        Key_ key;
        key.button_pane  = bp;
        key.text_pane    = tp;
        key.text         = tp->GetText();
        key.shifted_text = GetShiftedText_(key.text);

        const size_t index = keys_.size();
        if (key.text == "Shift") {
            bp->GetButton().GetClicked().AddObserver(
                this, [&, index](const ClickInfo &){
                    ProcessShiftKey_(keys_[index]); });
        }
        else {
            bp->GetButton().GetClicked().AddObserver(
                this, [&, index](const ClickInfo &){
                    ProcessKey_(keys_[index]); });
        }

        keys_.push_back(key);
    }
}

bool KeyboardPanel::HandleEvent(const Event &event) {
    // Track headset on/off sevents to set the VirtualKeyboard visibility.
    if ((event.flags.Has(Event::Flag::kButtonPress) ||
         event.flags.Has(Event::Flag::kButtonRelease)) &&
        event.button == Event::Button::kHeadset)
        virtual_keyboard_->SetIsVisible(
            event.flags.Has(Event::Flag::kButtonPress));

    return Panel::HandleEvent(event);
}

void KeyboardPanel::PaneActivated(Pane &pane) {
    // If this is a TextInputPane, activate and attach the VirtualKeyboard.
    if (auto *tip = dynamic_cast<TextInputPane *>(&pane)) {
        virtual_keyboard_->SetIsActive(true);
        tip->AttachToVirtualKeyboard(*virtual_keyboard_);
    }
}

void KeyboardPanel::PaneDeactivated(Pane &pane) {
    // If this is a TextInputPane, deactivate and detach the VirtualKeyboard.
    if (auto *tip = dynamic_cast<TextInputPane *>(&pane)) {
        tip->DetachFromVirtualKeyboard(*virtual_keyboard_);
        virtual_keyboard_->SetIsActive(false);
    }
}

void KeyboardPanel::FindButtonPanes_(const PanePtr &pane,
                                     std::vector<ButtonPanePtr> &button_panes) {
    if (ButtonPanePtr bp = Util::CastToDerived<ButtonPane>(pane))
        button_panes.push_back(bp);

    // Recurse if this is a ContainerPane.
    if (ContainerPanePtr ctr = Util::CastToDerived<ContainerPane>(pane)) {
        for (const auto &sub_pane: ctr->GetPanes())
            FindButtonPanes_(sub_pane, button_panes);
    }
}

void KeyboardPanel::ProcessShiftKey_(const Key_ &key) {
    auto &but = key.button_pane->GetButton();
    if (but.GetToggleState()) {
        if (shift_count_++ == 0)
            UpdateKeyText_(true);
    }
    else {
        ASSERT(shift_count_ > 0);
        if (--shift_count_ == 0)
            UpdateKeyText_(false);
    }
}

void KeyboardPanel::ProcessKey_(const Key_ &key) {
    // Special cases.
    if      (key.text == "Backspace")
        virtual_keyboard_->DeletePreviousChar();
    else if (key.text == "Clear")
        virtual_keyboard_->ClearText();
    else if (key.text == "Enter")
        virtual_keyboard_->Finish(true);
    else if (key.text == "Cancel")
        virtual_keyboard_->Finish(false);
    else
        virtual_keyboard_->InsertText(key.text);
}

void KeyboardPanel::UpdateKeyText_(bool is_shifted) {
    for (const auto &key: keys_) {
        if (! key.shifted_text.empty())
            key.text_pane->SetText(is_shifted ? key.shifted_text : key.text);
    }
}

std::string KeyboardPanel::GetShiftedText_(const std::string &s) {
    std::string ss;

    // Strings with more than one character stay the same; leave an empty
    // string to indicate this.

    if (s.size() == 1U) {
        // Single alphabetic characters can be converted to upper case.
        if (std::isalpha(s[0]))
            ss = Util::ToUpperCase(s);

        else {
            // Special case other single characters.
            switch (s[0]) {
              case '1':  ss = "!";  break;
              case '2':  ss = "@";  break;
              case '3':  ss = "#";  break;
              case '4':  ss = "$";  break;
              case '5':  ss = "%";  break;
              case '6':  ss = "^";  break;
              case '7':  ss = "&";  break;
              case '8':  ss = "*";  break;
              case '9':  ss = "(";  break;
              case '0':  ss = ")";  break;
              case '-':  ss = "_";  break;
              case '=':  ss = "+";  break;
              case '[':  ss = "{";  break;
              case ']':  ss = "}";  break;
              case '\\': ss = "|";  break;
              case ';':  ss = ":";  break;
              case '\'': ss = "\""; break;
              case ',':  ss = "<";  break;
              case '.':  ss = ">";  break;
              case '/':  ss = "?";  break;
              default: ASSERTM(false, "Unknown text: " + s);
            }
        }
    }
    return ss;
}
