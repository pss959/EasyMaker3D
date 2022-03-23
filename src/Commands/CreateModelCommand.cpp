#include "Commands/CreateModelCommand.h"

#include "Models/Model.h"
#include "Util/Assert.h"

void CreateModelCommand::AddFields() {
    AddField(result_name_);
    Command::AddFields();
}

bool CreateModelCommand::IsValid(std::string &details) {
    if (! Command::IsValid(details))
        return false;
    if (! Model::IsValidName(result_name_)) {
        details = "invalid model name";
        return false;
    }
    return true;
}

void CreateModelCommand::SetResultName(const std::string &name) {
    ASSERT(Model::IsValidName(name));
    result_name_ = name;
}
