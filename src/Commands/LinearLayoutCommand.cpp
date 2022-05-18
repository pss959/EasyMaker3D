#include "Commands/LinearLayoutCommand.h"

void LinearLayoutCommand::AddFields() {
    AddField(offset_);
    MultiModelCommand::AddFields();
}

std::string LinearLayoutCommand::GetDescription() const {
    return "Linear layout of " + GetModelsDesc(GetModelNames());
}
