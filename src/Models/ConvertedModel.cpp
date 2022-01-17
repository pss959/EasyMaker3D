#include "Models/ConvertedModel.h"

void ConvertedModel::AddFields() {
    AddField(original_model_);
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

void ConvertedModel::CreationDone(bool is_template) {
    ParentModel::CreationDone(is_template);

    // Add original model as a child and do not show it by default.
    if (! is_template) {
        auto &orig = GetOriginalModel();
        orig->SetStatus(Status::kAncestorShown);
        ParentModel::AddChildModel(orig);
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
