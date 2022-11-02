#include "Models/ConvertedModel.h"

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
    original_model_ = model;
    SyncTransformsFromOriginal(*model);
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

void ConvertedModel::SetStatus(Status status) {
    const Status prev_status = GetStatus();
    const bool   was_shown   = IsShown();
    ParentModel::SetStatus(status);

    // If the child was shown and now the ConvertedModel is, update the
    // transforms in the ConvertedModel from the original.
    if (prev_status == Status::kDescendantShown && IsShown()) {
        SyncTransformsFromOriginal(*GetOriginalModel());
    }

    // If the ConvertedModel was shown and now the child may be, update the
    // transforms in the original from the ConvertedModel.
    else if (was_shown && status == Status::kDescendantShown) {
        SyncTransformsToOriginal(*GetOriginalModel());
    }
}

void ConvertedModel::CopyContentsFrom(const Parser::Object &from,
                                      bool is_deep) {
    Model::CopyContentsFrom(from, is_deep);

    // Clone the original Model.
    const ConvertedModel &from_cm = static_cast<const ConvertedModel &>(from);
    SetOriginalModel(from_cm.GetOriginalModel()->CreateClone());
}

void ConvertedModel::SyncTransformsFromOriginal(const Model &original) {
    CopyTransformsFrom(original);
}

void ConvertedModel::SyncTransformsToOriginal(Model &original) const {
    original.CopyTransformsFrom(*this);
}
