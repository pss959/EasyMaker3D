#include "Commands/ChangeTextCommand.h"

#include "Math/TextUtils.h"
#include "Models/Model.h"
#include "Util/Assert.h"

void ChangeTextCommand::AddFields() {
    AddField(new_text_string_.Init("new_text_string"));
    AddField(new_font_name_.Init("new_font_name"));
    AddField(new_char_spacing_.Init("new_char_spacing", 1));

    MultiModelCommand::AddFields();
}

bool ChangeTextCommand::IsValid(Str &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (new_font_name_.WasSet() && ! IsValidFontName(new_font_name_)) {
        details = "Invalid new font name";
        return false;
    }
    if (new_text_string_.WasSet() && new_text_string_.GetValue().empty()) {
        details = "Empty new text string";
        return false;
    }
    return true;
}

Str ChangeTextCommand::GetDescription() const {
    return "Changed the text or options in " + GetModelsDesc(GetModelNames());
}

void ChangeTextCommand::SetNewTextString(const Str &new_text) {
    ASSERT(! new_text.empty());
    new_text_string_ = new_text;
}

void ChangeTextCommand::SetNewFontName(const Str &name) {
    ASSERT(IsValidFontName(name));
    new_font_name_ = name;
}

void ChangeTextCommand::SetNewCharSpacing(float spacing) {
    new_char_spacing_ = spacing;
}
