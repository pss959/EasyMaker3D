#include "Commands/TranslateCommand.h"

void TranslateCommand::AddFields() {
    AddField(translation_);
    MultiModelCommand::AddFields();
}

std::string TranslateCommand::GetDescription() const {
    return "Translated " + GetModelsDesc(GetModelNames());
}
