#include "Commands/ChangeNameCommand.h"

#include "Models/Model.h"
#include "Util/Assert.h"

void ChangeNameCommand::AddFields() {
    AddField(new_name_.Init("new_name"));

    SingleModelCommand::AddFields();
}

bool ChangeNameCommand::IsValid(std::string &details) {
    if (! SingleModelCommand::IsValid(details))
        return false;
    if (! Model::IsValidName(new_name_)) {
        details = "invalid new model name";
        return false;
    }
    return true;
}

std::string ChangeNameCommand::GetDescription() const {
    return "Changed the name of " + GetModelDesc(GetModelName());
}

void ChangeNameCommand::SetNewName(const std::string &new_name) {
    ASSERT(Model::IsValidName(new_name));
    new_name_ = new_name;
}
