#include "Commands/CombineCommand.h"

#include "Models/Model.h"

void CombineCommand::AddFields() {
    AddField(result_name_.Init("result_name"));

    MultiModelCommand::AddFields();
}

bool CombineCommand::IsValid(Str &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (! Model::IsValidName(result_name_)) {
        details = "Invalid result model name";
        return false;
    }
    return true;
}

void CombineCommand::SetResultName(const Str &name) {
    ASSERT(Model::IsValidName(name));
    result_name_ = name;
}

Str CombineCommand::BuildDescription(const Str &type) const {
    return "Created " + type + " model \"" + GetResultName() + "\" from " +
        GetModelsDesc(GetModelNames());
}
