#include "Commands/CreatePrimitiveModelCommand.h"

#include "Models/Model.h"
#include "Util/Enum.h"

void CreatePrimitiveModelCommand::AddFields() {
    Command::AddFields();
    AddField(type_);
    AddField(model_name_);
}

bool CreatePrimitiveModelCommand::IsValid(std::string &details) {
    if (! Command::IsValid(details))
        return false;
    if (! Model::IsValidName(model_name_)) {
        details = "invalid model name";
        return false;
    }
    return true;
}

std::string CreatePrimitiveModelCommand::GetDescription() const {
    return "Create a " + type_.GetEnumName() + " Model";
}

void CreatePrimitiveModelCommand::SetModelName(const std::string &name) {
    ASSERT(Model::IsValidName(name));
    model_name_ = name;
}
