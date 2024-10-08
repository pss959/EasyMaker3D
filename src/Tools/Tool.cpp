//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tools/Tool.h"

#include <ion/math/transformutils.h>

#include "Items/SessionState.h"
#include "Managers/BoardManager.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/SettingsManager.h"
#include "Managers/TargetManager.h"
#include "Math/Linear.h"
#include "Models/ConvertedModel.h"
#include "Models/Model.h"
#include "Models/RootModel.h"
#include "Place/PrecisionStore.h"
#include "SG/ColorMap.h"
#include "SG/CoordConv.h"
#include "SG/Search.h"
#include "Util/Assert.h"

void Tool::SetContext(const ContextPtr &context) {
    ASSERT(context);
    ASSERT(context->board_manager);
    ASSERT(context->command_manager);
    ASSERT(context->feedback_manager);
    ASSERT(context->precision_store);
    ASSERT(context->settings_manager);
    ASSERT(context->target_manager);
    ASSERT(context->root_model);
    ASSERT(context->board);
    ASSERT(! context->path_to_parent_node.empty());

    context_ = context;
}

void Tool::SetSpecializedCompletionFunc(const CompletionFunc &func) {
    ASSERT(IsSpecialized());
    completion_func_ = func;
}

bool Tool::CanBeUsedFor(const Selection &sel) const {
    return sel.HasAny() && CanAttach(sel);
}

void Tool::AttachToSelection(const Selection &sel, size_t index) {
    ASSERT(sel.HasAny());
    selection_         = sel;
    model_sel_index_ = index;
    Attach();
}

void Tool::DetachFromSelection() {
    ASSERT(selection_.HasAny());
    Detach();
    selection_.Clear();
    model_sel_index_ = -1;
}

void Tool::ReattachToSelection() {
    ASSERT(selection_.HasAny());
    ASSERT(model_sel_index_ >= 0);
    Detach();
    Attach();
}

ModelPtr Tool::GetModelAttachedTo() const {
    if (model_sel_index_ >= 0)
        return selection_.GetPaths()[model_sel_index_].GetModel();
    else
        return ModelPtr();
}

void Tool::Update() {
}

const SG::Node * Tool::GetGrippableNode() const {
    return GetModelAttachedTo() ? this : nullptr;
}

void Tool::UpdateGripInfo(GripInfo &info) {
    info.guide_type = GripGuideType::kNone;
    info.widget.reset();
}

Tool::Context & Tool::GetContext() const {
    ASSERT(context_);
    return *context_;
}

SG::CoordConv Tool::GetStageCoordConv() const {
    ASSERT(selection_.HasAny());
    ASSERT(model_sel_index_ >= 0);
    return SG::CoordConv(selection_.GetPaths()[model_sel_index_]);
}

Point3f Tool::ToWorld(const Point3f &p) const {
    ASSERT(! context_->path_to_parent_node.empty());
    return SG::CoordConv(context_->path_to_parent_node).ObjectToRoot(p);
}

Vector3f Tool::ToWorld(const Vector3f &v) const {
    ASSERT(! context_->path_to_parent_node.empty());
    return SG::CoordConv(context_->path_to_parent_node).ObjectToRoot(v);
}

Point3f Tool::ToWorld(const SG::NodePtr &local_node, const Point3f &p) const {
    const auto &path_to_parent = context_->path_to_parent_node;
    ASSERT(! path_to_parent.empty());
    auto path = SG::FindNodePathUnderNode(path_to_parent.back(), *local_node);
    ASSERT(! path.empty());
    auto full_path = SG::NodePath::Stitch(path_to_parent, path);
    return SG::CoordConv(full_path).ObjectToRoot(p);
}

Vector3f Tool::MatchModelAndGetSize(bool allow_axis_aligned) {
    ASSERT(GetModelAttachedTo());
    const Model &model = *GetModelAttachedTo();

    // Rotate to match the Model if not aligning.
    const bool align = allow_axis_aligned && IsAxisAligned();
    SetRotation(align ? Rotationf::Identity() : model.GetRotation());

    // Move the Tool to the center of the Model in stage coordinates.
    const Bounds &obj_bounds = model.GetBounds();
    const Matrix4f osm = GetStageCoordConv().GetObjectToRootMatrix();
    TranslateTo(osm * obj_bounds.GetCenter());

    // If aligned, use the size of the stage bounds for the Model.  Otherwise,
    // use the scale (including all ancestors) applied to the size of the
    // object bounds.
    return align ? TransformBounds(obj_bounds, osm).GetSize() :
        ion::math::GetScaleVector(osm) * obj_bounds.GetSize();
}

Vector3f Tool::MatchOperandModelAndGetSize(bool use_operand_model_size) {
    ConvertedModelPtr cm =
        std::dynamic_pointer_cast<ConvertedModel>(GetModelAttachedTo());
    ASSERT(cm);

    // Always rotate to align with the ConvertedModel.
    SetRotation(cm->GetRotation());

    // Translate the Tool so that it is centered on the operand mesh.
    const Bounds &operand_bounds = cm->GetOperandModel()->GetBounds();
    const Matrix4f osm = GetStageCoordConv().GetObjectToRootMatrix();
    TranslateTo(osm * operand_bounds.GetCenter() - cm->GetLocalCenterOffset());

    // Return the size of the ConvertedModel's or operand Model's mesh in stage
    // coordinates.
    const Vector3f bounds_size = use_operand_model_size ?
        operand_bounds.GetSize() : cm->GetBounds().GetSize();

    return ion::math::GetScaleVector(osm) * bounds_size;
}

bool Tool::IsAxisAligned() const {
    return context_->command_manager->GetSessionState()->IsAxisAligned();
}

int Tool::SnapToAxis(Vector3f &dir) {
    auto &tm = *GetContext().target_manager;

    const Matrix4f m = IsAxisAligned() ? Matrix4f::Identity() :
        GetStageCoordConv().GetObjectToRootMatrix();

    int snapped_dim = -1;

    Rotationf rot;
    for (int dim = 0; dim < 3; ++dim) {
        const Vector3f axis = ion::math::Normalized(m * GetAxis(dim));
        if (tm.ShouldSnapDirections(dir, axis, rot)) {
            dir = axis;
            snapped_dim = dim;
            break;
        }
        else if (tm.ShouldSnapDirections(dir, -axis, rot)) {
            dir = -axis;
            snapped_dim = dim;
            break;
        }
    }
    return snapped_dim;
}

Point3f Tool::GetPositionAboveModel(float distance) const {
    // Get the point at the top center of the Model in stage coordinates.
    const Bounds stage_bounds = GetStageBounds();
    Point3f pos = stage_bounds.GetCenter();
    pos[1] = stage_bounds.GetMaxPoint()[1] + distance;
    return pos;
}

Bounds Tool::GetStageBounds() const {
    return TransformBounds(GetModelAttachedTo()->GetBounds(),
                           GetStageCoordConv().GetObjectToRootMatrix());
}

float Tool::ComputePartScale(const Vector3f &model_size, float fraction,
                             float min_size, float max_size) {
    // Average the sizes.
    const float avg_size = (model_size[0] + model_size[1] + model_size[2]) / 3;
    return Clamp(fraction * avg_size, min_size, max_size);
}

Color Tool::GetNeutralFeedbackColor() {
    return SG::ColorMap::SGetColor("FeedbackNeutralColor");
}

Color Tool::GetSnappedFeedbackColor() {
    return SG::ColorMap::SGetColor("TargetActiveColor");
}

Color Tool::GetFeedbackColor(int dim, bool is_snapped) {
    // If snapped, use the snapped color. Otherwise, use a lighter version of
    // the dimension color.
    return is_snapped ? GetSnappedFeedbackColor() :
        Lerp(.25f, SG::ColorMap::SGetColorForDimension(dim), Color::White());
}

void Tool::Finish() {
    ASSERT(IsSpecialized());
    if (completion_func_)
        completion_func_();
}
