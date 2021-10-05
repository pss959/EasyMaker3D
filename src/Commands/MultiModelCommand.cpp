#include "Commands/MultiModelCommand.h"

void MultiModelCommand::AddFields() {
    AddField(model_names_);
    Command::AddFields();
}

bool MultiModelCommand::IsValid(std::string &details) {
    if (! Command::IsValid(details))
        return false;

    if (GetModelNames().empty()) {
        details = "missing model names";
        return false;
    }

    return true;
}
