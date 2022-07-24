#include "Commands/CreatePrimitiveModelCommand.h"

#include "Util/Enum.h"

void CreatePrimitiveModelCommand::AddFields() {
    AddField(type_.Init("type", PrimitiveType::kBox));

    CreateModelCommand::AddFields();
}

std::string CreatePrimitiveModelCommand::GetDescription() const {
    return "Create " + type_.GetEnumWords() + " Model " + GetResultName();
}
