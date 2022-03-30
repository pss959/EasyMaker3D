#include "Commands/CreateTextModelCommand.h"

#include "Util/Enum.h"

void CreateTextModelCommand::AddFields() {
    AddField(text_);
    CreateModelCommand::AddFields();
}

std::string CreateTextModelCommand::GetDescription() const {
    return "Create a 3D Text Model";
}
