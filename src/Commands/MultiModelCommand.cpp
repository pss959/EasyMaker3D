#include "Commands/MultiModelCommand.h"

#include "Models/Model.h"
#include "Util/Assert.h"
#include "Util/General.h"

void MultiModelCommand::AddFields() {
    AddField(model_names_.Init("model_names"));

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
    SetModelNames(
        Util::ConvertVector<std::string, SelPath>(
            sel.GetPaths(),
            [](const SelPath &path){ return path.GetModel()->GetName(); }));
}
