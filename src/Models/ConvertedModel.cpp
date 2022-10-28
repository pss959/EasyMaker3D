#include "Models/ConvertedModel.h"

#include "Math/MeshUtils.h"

void ConvertedModel::AddFields() {
    AddModelField(original_model_.Init("original_model"));

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
    if (GetChildModelCount() > 0)
        ClearChildModels();
    ParentModel::AddChildModel(model);
    UpdateOriginalModel_(model);
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

TriMesh ConvertedModel::BuildMesh() {
    auto orig = GetOriginalModel();
    ASSERT(orig);

    // Let the derived class convert the transformed chid mesh.
    return ConvertMesh(TransformMesh(orig->GetMesh(), orig->GetModelMatrix()));
}

void ConvertedModel::CopyContentsFrom(const Parser::Object &from,
                                      bool is_deep) {
    Model::CopyContentsFrom(from, is_deep);

    // Clone the original Model.
    const ConvertedModel &from_cm = static_cast<const ConvertedModel &>(from);
    SetOriginalModel(from_cm.GetOriginalModel()->CreateClone());
}

void ConvertedModel::UpdateOriginalModel_(const ModelPtr &model) {
    original_model_ = model;

    // Copy the transformation from the original.
    // XXXX
}
