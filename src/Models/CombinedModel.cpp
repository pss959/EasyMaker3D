#include "Models/CombinedModel.h"

#include "Math/MeshUtils.h"
#include "Util/String.h"

void CombinedModel::AddFields() {
    AddModelField(operand_models_.Init("operand_models"));

    ParentModel::AddFields();
}

bool CombinedModel::IsValid(std::string &details) {
    if (! ParentModel::IsValid(details))
        return false;
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
            ParentModel::AddChildModel(model);
        }
    }

    ParentModel::CreationDone();
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
    operand_models_.Add(child);
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

bool CombinedModel::ProcessChange(SG::Change change, const Object &obj) {
    if (! ParentModel::ProcessChange(change, obj)) {
        return false;
    }
    else {
        // A change in transform to a child or other descendent should cause
        // the Mesh to be rebuilt.
        if (change == SG::Change::kTransform && &obj != this)
            MarkMeshAsStale();
        return true;
    }
}

TriMesh CombinedModel::BuildMesh() {
    TriMesh mesh = CombineMeshes(GetChildMeshes());
    center_offset_ = Vector3f(ComputeMeshBounds(mesh).GetCenter());
    return CenterMesh(mesh);
}
