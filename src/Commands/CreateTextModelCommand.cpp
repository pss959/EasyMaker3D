#include "Commands/CreateTextModelCommand.h"

#include "Util/Enum.h"

void CreateTextModelCommand::AddFields() {
    AddField(text_.Init("text", "A"));

    CreateModelCommand::AddFields();
}

std::string CreateTextModelCommand::GetDescription() const {
    return BuildDescription("3D text");
}
