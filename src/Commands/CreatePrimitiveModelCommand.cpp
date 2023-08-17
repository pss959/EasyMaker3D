#include "Commands/CreatePrimitiveModelCommand.h"

#include "Util/Enum.h"

void CreatePrimitiveModelCommand::AddFields() {
    AddField(type_.Init("type", PrimitiveType::kBox));

    CreateModelCommand::AddFields();
}

Str CreatePrimitiveModelCommand::GetDescription() const {
    return BuildDescription(type_.GetEnumWords());
}
