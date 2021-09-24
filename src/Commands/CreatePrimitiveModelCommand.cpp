#include "Commands/CreatePrimitiveModelCommand.h"

#include "Util/Enum.h"

void CreatePrimitiveModelCommand::AddFields() {
    Command::AddFields();
    AddField(type_);
    AddField(model_name_);
}

bool CreatePrimitiveModelCommand::IsValid(std::string &details) {
    if (! Command::IsValid(details))
        return false;
    if (model_name_.GetValue().empty()) {
        details = "Missing model name";
        return false;
    }
    return true;
}

std::string CreatePrimitiveModelCommand::GetDescription() const {
    return "Create a " + type_.GetEnumName() + " Model";
}
