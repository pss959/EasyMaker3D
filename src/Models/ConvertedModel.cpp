#include "Models/ConvertedModel.h"

void ConvertedModel::AddFields() {
    AddModelField(operand_model_.Init("operand_model"));

    OperatorModel::AddFields();
}

bool ConvertedModel::IsValid(std::string &details) {
    if (! OperatorModel::IsValid(details))
        return false;
    if (! operand_model_.GetValue()) {
        details = "No operand model specified";
        return false;
    }
    return true;
}

void ConvertedModel::CreationDone() {
    OperatorModel::CreationDone();

    // Add operand model (if it exists) as a child and do not show it by
    // default. No need to do this if this is a clone.
    if (! IsTemplate() && ! IsClone()) {
        if (auto &operand = GetOperandModel()) {
            ASSERT(GetChildModelCount() == 0);
            operand->SetStatus(Status::kAncestorShown);
            OperatorModel::AddChildModel(operand);
        }
    }
}

void ConvertedModel::SetOperandModel(const ModelPtr &model) {
    ASSERT(model);
    if (GetChildModelCount() > 0)
        ClearChildModels();
    OperatorModel::AddChildModel(model);
    operand_model_ = model;
}

void ConvertedModel::AddChildModel(const ModelPtr &child) {
    // This should never be called.
    ASSERTM(false, "ConvertedModel::AddChildModel() should not be called");
}

void ConvertedModel::InsertChildModel(size_t index, const ModelPtr &child) {
    // This should never be called.
    ASSERTM(false, "ConvertedModel::InsertChildModel() should not be called");
}

void ConvertedModel::RemoveChildModel(size_t index) {
    // This should never be called.
    ASSERTM(false, "ConvertedModel::RemoveChildModel() should not be called");
}

void ConvertedModel::ReplaceChildModel(size_t index, const ModelPtr &new_child) {
    // This should never be called.
    ASSERTM(false, "ConvertedModel::ReplaceChildModel() should not be called");
}

TriMesh ConvertedModel::BuildMeshFromOperands() {
    const auto meshes = GetChildMeshes();
    ASSERT(meshes.size() == 1U);
    return ConvertMesh(meshes[0]);
}
