//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Models/RootModel.h"

#include "Util/KLog.h"

void RootModel::CreationDone() {
    ParentModel::CreationDone();
    // RootModel always has this status so its uniforms are processed and its
    // descendents are visible.
    SetStatus(Status::kUnselected);
}

void RootModel::UpdateAddedChildModel(Model &child) {
    ParentModel::UpdateAddedChildModel(child);
    child.SetUse(Model::Use::kInScene);
    child.SetStatus(Model::Status::kUnselected);
    top_level_changed_.Notify();
}

void RootModel::UpdateRemovedChildModel(Model &child) {
    ParentModel::UpdateRemovedChildModel(child);
    top_level_changed_.Notify();
}

void RootModel::Reset() {
    ClearChildModels();
}

void RootModel::UpdateGlobalUniforms(const Matrix4f &wsm,
                                     const Vector3f &bv_size) {
    auto &block = GetUniformBlockForPass("Lighting");
    block.SetMatrix4fUniformValue("uWorldToStageMatrix", wsm);
    block.SetVector3fUniformValue("uBuildVolumeSize",    bv_size);
}

void RootModel::ShowEdges(bool show) {
    if (show != are_edges_shown_) {
        are_edges_shown_ = show;
        auto &block = GetUniformBlockForPass("Lighting");
        block.SetFloatUniformValue("uEdgeWidth", show ? 1 : 0);
        ProcessChange(SG::Change::kAppearance, *this);
    }
}

void RootModel::SetModelVisibility(const ModelPtr &model, bool is_visible) {
    // If model is null, change all top-level Models.
    if (! model) {
        for (size_t i = 0; i < GetChildModelCount(); ++i) {
            auto child = GetChildModel(i);
            const auto is_now_visible =
                child->GetStatus() != Model::Status::kHiddenByUser;
            if (is_visible != is_now_visible)
                SetModelVisibility(child, is_visible);
        }
    }
    else {
        ASSERT(model->IsTopLevel());
        if (is_visible) {
            ASSERT(model->GetStatus() == Model::Status::kHiddenByUser);
            KLOG('M', "Showing " << model->GetDesc());
            model->SetStatus(Model::Status::kUnselected);
        }
        else {
            ASSERT(model->GetStatus() != Model::Status::kHiddenByUser);
            ASSERT(! model->IsSelected());  // Error to hide a selected Model.
            KLOG('M', "Hiding " << model->GetDesc());
            model->SetStatus(Model::Status::kHiddenByUser);
        }
        top_level_changed_.Notify();
    }
}

size_t RootModel::GetHiddenModelCount() {
    const size_t count = GetChildModelCount();
    size_t hidden_count = 0;
    for (size_t i = 0; i < count; ++i)
        if (GetChildModel(i)->GetStatus() == Model::Status::kHiddenByUser)
            ++hidden_count;
    return hidden_count;
}
