#include "Tools/Tool.h"

#include "CoordConv.h"
#include "SG/Search.h"
#include "Util/Assert.h"

void Tool::SetContext(const ContextPtr &context) {
    ASSERT(context);
    ASSERT(context->command_manager);
    ASSERT(context->precision_manager);
    ASSERT(context->feedback_manager);

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

Matrix4f Tool::GetObjectToStageMatrix() const {
    ASSERT(selection_.HasAny());
    ASSERT(model_sel_index_ >= 0);
    return selection_.GetPaths()[model_sel_index_].GetObjectToStageMatrix();
}

Matrix4f Tool::GetLocalToStageMatrix() const {
    ASSERT(selection_.HasAny());
    ASSERT(model_sel_index_ >= 0);
    return selection_.GetPaths()[model_sel_index_].GetLocalToStageMatrix();
}

Point3f Tool::ToWorld(const SG::NodePtr &local_node, const Point3f &p) const {
    const auto &path_to_parent = context_->path_to_parent_node;
    ASSERT(! path_to_parent.empty());
    auto path = SG::FindNodePathUnderNode(path_to_parent.back(), local_node);
    auto full_path = SG::NodePath::Stitch(path_to_parent, path);
    return CoordConv().LocalToWorld(full_path, p);
}
