#include "Commands/MultiModelCommand.h"

#include "Models/Model.h"
#include "Util/Assert.h"

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
    for (const auto &name: GetModelNames()) {
        if (! Model::IsValidName(name)) {
            details = "invalid model name: '" + name + "'";
            return false;
        }
    }
    return true;
}

void MultiModelCommand::SetFromSelection(const Selection &sel) {
    ASSERT(sel.HasAny());
    const auto &paths = sel.GetPaths();
    std::vector<std::string> names;
    names.reserve(paths.size());
    for (const auto &path: paths)
        names.push_back(path.GetModel()->GetName());
    model_names_ = names;
}
