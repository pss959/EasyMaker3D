#include "Models/ConvertedModel.h"

#include "Math/MeshUtils.h"

void ConvertedModel::AddFields() {
    AddModelField(original_model_);
    ParentModel::AddFields();
}

bool ConvertedModel::IsValid(std::string &details) {
    if (! ParentModel::IsValid(details))
        return false;
    if (! original_model_.GetValue()) {
        details = "No original model specified";
        return false;
    }
    return true;
}

void ConvertedModel::CreationDone() {
    ParentModel::CreationDone();

    // Add original model (if it exists) as a child and do not show it by
    // default. No need to do this if this is a clone.
    if (! IsTemplate() && ! IsClone()) {
        if (auto &orig = GetOriginalModel()) {
            ASSERT(GetChildModelCount() == 0);
            orig->SetStatus(Status::kAncestorShown);
            ParentModel::AddChildModel(orig);
        }
    }
}

void ConvertedModel::SetOriginalModel(const ModelPtr &model) {
    ASSERT(model);
    original_model_ = model;
    if (GetChildModelCount() == 0)
        ParentModel::AddChildModel(model);
    else
        ParentModel::ReplaceChildModel(0, model);
}

void ConvertedModel::AddChildModel(const ModelPtr &child) {
    ASSERT(GetChildModelCount() == 0);
    ParentModel::AddChildModel(child);
    original_model_ = child;
}

void ConvertedModel::InsertChildModel(size_t index, const ModelPtr &child) {
    ASSERT(GetChildModelCount() == 0);
    AddChildModel(child);
}

void ConvertedModel::RemoveChildModel(size_t index) {
    ParentModel::RemoveChildModel(index);
    if (GetChildModelCount() == 0)
        original_model_ = ModelPtr();
}

void ConvertedModel::ReplaceChildModel(size_t index, const ModelPtr &new_child) {
    ParentModel::ReplaceChildModel(index, new_child);
    original_model_ = new_child;
}

TriMesh ConvertedModel::BuildMesh() {
    auto orig = GetOriginalModel();
    ASSERT(orig);

    // Let the derived class convert the transformed chid mesh.
    return ConvertMesh(TransformMesh(orig->GetMesh(), orig->GetModelMatrix()));
}
