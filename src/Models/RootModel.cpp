#include "Models/RootModel.h"

void RootModel::UpdateAddedChildModel(Model &child) {
    ParentModel::UpdateAddedChildModel(child);
    child.SetUse(Model::Use::kInScene);
    child.SetStatus(Model::Status::kUnselected);
}

void RootModel::Reset() {
    ClearChildModels();
}

void RootModel::HideModel(const ModelPtr &model) {
    ASSERT(model);
    ASSERT(model->IsTopLevel());
    ASSERT(model->GetStatus() != Model::Status::kHiddenByUser);
    model->SetStatus(Model::Status::kHiddenByUser);
    top_level_changed_.Notify();
}

void RootModel::ShowModel(const ModelPtr &model) {
    ASSERT(model);
    ASSERT(model->IsTopLevel());
    ASSERT(model->GetStatus() == Model::Status::kHiddenByUser);
    model->SetStatus(Model::Status::kUnselected);
    top_level_changed_.Notify();
}

size_t RootModel::GetHiddenModelCount() {
    const size_t count = GetChildModelCount();
    size_t hidden_count = 0;
    for (size_t i = 0; i < count; ++i)
        if (GetChildModel(i)->GetStatus() == Model::Status::kHiddenByUser)
            ++hidden_count;
    return hidden_count;
}

void RootModel::ShowAllModels() {
    const size_t count = GetChildModelCount();
    for (size_t i = 0; i < count; ++i) {
        auto child = GetChildModel(i);
        if (child->GetStatus() == Model::Status::kHiddenByUser)
            ShowModel(child);
    }
}
