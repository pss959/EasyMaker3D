#include "Commands/PasteCommand.h"

#include "Models/Model.h"
#include "Util/Assert.h"
#include "Util/General.h"

void PasteCommand::AddFields() {
    AddField(parent_name_.Init("parent_name"));

    Command::AddFields();
}

std::string PasteCommand::GetDescription() const {
    ASSERTM(! model_names_.empty(),
            "SetModelsForDescription() was never called");
    std::string desc = "Pasted " + GetModelsDesc(model_names_);
    const auto &parent = parent_name_.GetValue();
    if (! parent.empty())
        desc += " into Model \"" + parent + "\"";
    return desc;
}

void PasteCommand::SetModelsForDescription(
    const std::vector<ModelPtr> &models) {
    model_names_ = Util::ConvertVector<std::string, ModelPtr>(
        models, [](const ModelPtr &m){ return m->GetName(); });
}
