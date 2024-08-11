//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Models/CombinedModel.h"

#include "Math/MeshUtils.h"
#include "Util/General.h"
#include "Util/String.h"

void CombinedModel::AddFields() {
    AddModelField(operand_models_.Init("operand_models"));

    OperatorModel::AddFields();
}

bool CombinedModel::IsValid(Str &details) {
    if (! OperatorModel::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
    const size_t min = GetMinChildCount();
    if (GetOperandModels().size() < min) {
        details = "Only " + Util::ToString(GetOperandModels().size()) +
            " operand model(s); at least " + Util::ToString(min) + " required";
        return false;
    }
    return true;
}

void CombinedModel::CreationDone() {
    // Add operand models as children first. Also, do not show the child models
    // by default. No need to do this if this is a clone.
    if (! IsTemplate() && ! IsClone()) {
        for (auto &model: GetOperandModels()) {
            model->SetStatus(Status::kAncestorShown);
            OperatorModel::AddChildModel(model);
        }
    }

    OperatorModel::CreationDone();
}

void CombinedModel::SetOperandModels(std::vector<ModelPtr> models) {
    operand_models_.GetValue() = models;
    for (auto &model: models) {
        model->SetStatus(Status::kAncestorShown);
        OperatorModel::AddChildModel(model);
    }
}

void CombinedModel::AddChildModel(const ModelPtr &child) {
    OperatorModel::AddChildModel(child);
    operand_models_.Add(child);
}

void CombinedModel::InsertChildModel(size_t index, const ModelPtr &child) {
    OperatorModel::InsertChildModel(index, child);
    auto &opmodels = operand_models_.GetValue();
    if (index >= opmodels.size())
        opmodels.push_back(child);
    else
        opmodels.insert(opmodels.begin() + index, child);
}

void CombinedModel::RemoveChildModel(size_t index) {
    OperatorModel::RemoveChildModel(index);
    auto &opmodels = operand_models_.GetValue();
    opmodels.erase(opmodels.begin() + index);
}

void CombinedModel::ReplaceChildModel(size_t index, const ModelPtr &new_child) {
    OperatorModel::ReplaceChildModel(index, new_child);
    ModelPtr child = GetChildModel(index);
    operand_models_.GetValue()[index] = child;
}

void CombinedModel::CopyContentsFrom(const Parser::Object &from, bool is_deep) {
    OperatorModel::CopyContentsFrom(from, is_deep);

    // Clone the operand Models.
    const CombinedModel &from_cm = static_cast<const CombinedModel &>(from);
    SetOperandModels(Util::ConvertVector<ModelPtr, ModelPtr>(
                         from_cm.GetOperandModels(),
                         [&](const ModelPtr &m){ return m->CreateClone(); }));
}

TriMesh CombinedModel::BuildMeshFromOperands() {
    return CombineMeshes(GetChildMeshes());
}
