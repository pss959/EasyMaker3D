#include "Panes/KeyPane.h"

#include "Panes/IconPane.h"
#include "Panes/TextPane.h"
#include "Util/Assert.h"
#include "Util/General.h"

void KeyPane::AddFields() {
    AddField(text_);
    AddField(shifted_text_);
    AddField(icon_name_);
    ButtonPane::AddFields();
}

bool KeyPane::IsValid(std::string &details) {
    if (! ButtonPane::IsValid(details))
        return false;

    if (text_.GetValue().empty()) {
        details = "No text specified";
        return false;
    }

    return true;
}

void KeyPane::CreationDone() {
    ButtonPane::CreationDone();

    if (! IsTemplate()) {
        // Copy text to shifted_text_ if not present.
        if (shifted_text_.GetValue().empty())
            shifted_text_ = text_;

        // Access the TextPane and IconPane.
        auto text_pane = FindTypedPane<TextPane>("Text");
        auto icon_pane = FindTypedPane<IconPane>("Icon");

        // Disable the IconPane or TextPane depending on whether there is an
        // icon name.
        if (icon_name_.GetValue().empty()) {
            text_pane->SetText(text_);
            icon_pane->SetEnabled(false);
        }
        else {
            icon_pane->SetIconName(icon_name_);
            text_pane->SetEnabled(false);
        }
    }
}
