#include "Commands/TranslateCommand.h"

void TranslateCommand::AddFields() {
    AddField(translation_.Init("translation", Vector3f::Zero()));

    MultiModelCommand::AddFields();
}

std::string TranslateCommand::GetDescription() const {
    return "Translated " + GetModelsDesc(GetModelNames());
}
