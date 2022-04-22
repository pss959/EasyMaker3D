#include "Commands/PasteCommand.h"

#include "Models/Model.h"

void PasteCommand::AddFields() {
    AddField(parent_name_);
    Command::AddFields();
}

std::string PasteCommand::GetDescription() const {
    return "Pasted " + GetModelsDesc(model_names_);
}

void PasteCommand::SetModelsForDescription(
    const std::vector<ModelPtr> &models) {
    for (const auto &model: models)
        model_names_.push_back(model->GetName());
}
