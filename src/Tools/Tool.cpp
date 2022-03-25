#include "Tools/Tool.h"

#include <ion/math/transformutils.h>

#include "CoordConv.h"
#include "Managers/ColorManager.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/PanelManager.h"
#include "Managers/PrecisionManager.h"
#include "Managers/SettingsManager.h"
#include "Managers/TargetManager.h"
#include "Models/Model.h"
#include "SG/Search.h"
#include "Util/Assert.h"

void Tool::SetContext(const ContextPtr &context) {
    ASSERT(context);
    ASSERT(context->color_manager);
    ASSERT(context->command_manager);
    ASSERT(context->precision_manager);
    ASSERT(context->feedback_manager);
    ASSERT(context->settings_manager);
    ASSERT(context->target_manager);
    ASSERT(context->panel_manager);

    context_ = context;
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

Tool::Context & Tool::GetContext() const {
    ASSERT(context_);
    return *context_;
}

CoordConv Tool::GetStageCoordConv() const {
    ASSERT(selection_.HasAny());
    ASSERT(model_sel_index_ >= 0);
    return CoordConv(selection_.GetPaths()[model_sel_index_]);
}

Point3f Tool::ToWorld(const SG::NodePtr &local_node, const Point3f &p) const {
    const auto &path_to_parent = context_->path_to_parent_node;
    ASSERT(! path_to_parent.empty());
    auto path = SG::FindNodePathUnderNode(path_to_parent.back(), *local_node);
    auto full_path = SG::NodePath::Stitch(path_to_parent, path);
    return CoordConv(full_path).LocalToRoot(p);
}

Vector3f Tool::MatchModelAndGetSize(bool allow_axis_aligned) {
    ASSERT(GetModelAttachedTo());
    const Model &model = *GetModelAttachedTo();

    // Rotate to match the Model if not aligning.
    const bool align = allow_axis_aligned && context_->is_axis_aligned;
    SetRotation(align ? Rotationf::Identity() : model.GetRotation());

    // Move the Tool to the center of the Model in stage coordinates.
    const Bounds &obj_bounds = model.GetBounds();
    const Matrix4f osm = GetStageCoordConv().GetObjectToRootMatrix();
    SetTranslation(osm * obj_bounds.GetCenter());

    // If aligned, use the size of the stage bounds for the Model.  Otherwise,
    // use the scale (including all ancestors) applied to the size of the
    // object bounds.
    return align ? TransformBounds(obj_bounds, osm).GetSize() :
        osm * obj_bounds.GetSize();
}

Vector3f Tool::GetPositionAboveModel(float distance) const {
    ASSERT(GetModelAttachedTo());
    const Point3f model_top = GetStageCoordConv().ObjectToRoot(
        GetModelAttachedTo()->GetBounds().GetFaceCenter(Bounds::Face::kTop));
    return Vector3f(model_top[0], model_top[1] + distance, model_top[2]);
}

Color Tool::GetSnappedFeedbackColor() {
    return ColorManager::GetSpecialColor("TargetActiveColor");
}

Color Tool::GetFeedbackColor(int dim, bool is_snapped) {
    return is_snapped ? GetSnappedFeedbackColor() :
        ColorManager::GetColorForDimension(dim);
}
