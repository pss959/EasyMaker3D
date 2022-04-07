#include "Commands/ChangeTextCommand.h"

#include "Math/TextUtils.h"
#include "Models/Model.h"
#include "Util/Assert.h"

void ChangeTextCommand::AddFields() {
    AddField(new_text_string_);
    AddField(new_font_name_);
    AddField(new_char_spacing_);
    MultiModelCommand::AddFields();
}

bool ChangeTextCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    // XXXX Validate text.
    if (new_font_name_.WasSet() && ! IsValidFontName(new_font_name_)) {
        details = "invalid new font name";
        return false;
    }
    return true;
}

std::string ChangeTextCommand::GetDescription() const {
    return "Changed the text or options in " + GetModelsDesc(GetModelNames());
}

void ChangeTextCommand::SetNewTextString(const std::string &new_text) {
    // XXXX Validate text.
    new_text_string_ = new_text;
}

void ChangeTextCommand::SetNewFontName(const std::string &name) {
    ASSERT(IsValidFontName(name));
    new_font_name_ = name;
}

void ChangeTextCommand::SetNewCharSpacing(float spacing) {
    new_char_spacing_ = spacing;
}
