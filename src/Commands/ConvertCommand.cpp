#include "Commands/ConvertCommand.h"

#include "Models/Model.h"

void ConvertCommand::AddFields() {
    AddField(result_names_.Init("result_names"));

    MultiModelCommand::AddFields();
}

bool ConvertCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;

    if (GetResultNames().empty()) {
        details = "missing result names";
        return false;
    }
    for (const auto &name: GetResultNames()) {
        if (! Model::IsValidName(name)) {
            details = "invalid result model name: '" + name + "'";
            return false;
        }
    }
    return true;
}
