#include "Commands/SingleModelCommand.h"

#include "Models/Model.h"
#include "Util/Assert.h"

void SingleModelCommand::AddFields() {
    AddField(model_name_.Init("model_name"));

    Command::AddFields();
}

bool SingleModelCommand::IsValid(std::string &details) {
    if (! Command::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
    if (! Model::IsValidName(model_name_)) {
        details = "Invalid model name";
        return false;
    }
    return true;
}

void SingleModelCommand::SetFromSelection(const Selection &sel) {
    ASSERT(sel.GetCount() == 1U);
    model_name_ = sel.GetPrimary().GetModel()->GetName();
}
