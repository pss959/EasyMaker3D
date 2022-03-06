#include "Commands/CombineCommand.h"

#include "Models/Model.h"

void CombineCommand::AddFields() {
    MultiModelCommand::AddFields();
    AddField(result_name_);
}

bool CombineCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (! Model::IsValidName(result_name_)) {
        details = "invalid result model name";
        return false;
    }
    return true;
}

void CombineCommand::SetResultName(const std::string &name) {
    ASSERT(Model::IsValidName(name));
    result_name_ = name;
}
