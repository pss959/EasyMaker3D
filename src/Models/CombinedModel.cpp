#include "Models/CombinedModel.h"

void CombinedModel::AddFields() {
    AddField(operand_models_);
    ParentModel::AddFields();
}

bool CombinedModel::IsValid(std::string &details) {
    if (! ParentModel::IsValid(details))
        return false;

    // Add operand models as children. Also, do not show the child models by
    // default.
    for (auto &model: GetOperandModels()) {
        model->SetStatus(Status::kAncestorShown);
        ParentModel::AddChildModel(model);
    }

    return true;
}

void CombinedModel::SetOperandModels(std::vector<ModelPtr> models) {
    operand_models_.GetValue() = models;
    for (auto &model: models) {
        model->SetStatus(Status::kAncestorShown);
        ParentModel::AddChildModel(model);
    }
}

void CombinedModel::AddChildModel(const ModelPtr &child) {
    ParentModel::AddChildModel(child);
    operand_models_.GetValue().push_back(child);
}

void CombinedModel::InsertChildModel(size_t index, const ModelPtr &child) {
    ParentModel::InsertChildModel(index, child);
    auto &opmodels = operand_models_.GetValue();
    if (index >= opmodels.size())
        opmodels.push_back(child);
    else
        opmodels.insert(opmodels.begin() + index, child);
}

void CombinedModel::RemoveChildModel(size_t index) {
    ParentModel::RemoveChildModel(index);
    auto &opmodels = operand_models_.GetValue();
    opmodels.erase(opmodels.begin() + index);
}

void CombinedModel::ReplaceChildModel(size_t index, const ModelPtr &new_child) {
    ParentModel::ReplaceChildModel(index, new_child);
    ModelPtr child = GetChildModel(index);
    operand_models_.GetValue()[index] = child;
}
