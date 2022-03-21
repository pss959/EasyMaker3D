#include "Commands/ChangeColorCommand.h"

void ChangeColorCommand::AddFields() {
    AddField(new_color_);
    MultiModelCommand::AddFields();
}

std::string ChangeColorCommand::GetDescription() const {
    return "Changed the color of " + GetModelsDesc(GetModelNames());
}
