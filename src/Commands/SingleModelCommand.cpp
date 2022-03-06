#include "Commands/SingleModelCommand.h"

#include "Models/Model.h"
#include "Util/Assert.h"

void SingleModelCommand::AddFields() {
    AddField(model_name_);
    Command::AddFields();
}

bool SingleModelCommand::IsValid(std::string &details) {
    if (! Command::IsValid(details))
        return false;

    if (! Model::IsValidName(model_name_)) {
        details = "invalid model name";
        return false;
    }

    return true;
}

void SingleModelCommand::SetFromSelection(const Selection &sel) {
    ASSERT(sel.GetCount() == 1U);
    model_name_ = sel.GetPrimary().GetModel()->GetName();
}
