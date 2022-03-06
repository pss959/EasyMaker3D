#include "Commands/CreatePrimitiveModelCommand.h"

#include "Models/Model.h"
#include "Util/Enum.h"

void CreatePrimitiveModelCommand::AddFields() {
    Command::AddFields();
    AddField(type_);
    AddField(result_name_);
}

bool CreatePrimitiveModelCommand::IsValid(std::string &details) {
    if (! Command::IsValid(details))
        return false;
    if (! Model::IsValidName(result_name_)) {
        details = "invalid model name";
        return false;
    }
    return true;
}

std::string CreatePrimitiveModelCommand::GetDescription() const {
    return "Create a " + type_.GetEnumName() + " Model";
}

void CreatePrimitiveModelCommand::SetResultName(const std::string &name) {
    ASSERT(Model::IsValidName(name));
    result_name_ = name;
}
