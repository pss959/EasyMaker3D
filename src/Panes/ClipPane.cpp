#include "Panes/ClipPane.h"

#include "Math/Linear.h"
#include "SG/Search.h"

void ClipPane::CreationDone() {
    BoxPane::CreationDone();

    if (! IsTemplate())
        clip_node_ = SG::FindNodeUnderNode(*this, "ClipNode");
}

Vector2f ClipPane::GetUnclippedSize() const {
    return BoxPane::ComputeBaseSize();
}

void ClipPane::SetContentsOffset(const Vector2f &offset) {
    auto &contents = GetContentsNode_();
    const Vector3f trans = contents.GetTranslation();
    contents.SetTranslation(Vector3f(offset[0], offset[1], trans[2]));
}

Vector2f ClipPane::GetContentsOffset() const {
    const Vector3f &trans = GetContentsNode_().GetTranslation();
    return Vector2f(trans[0], trans[1]);
}

bool ClipPane::IsSubPaneTouchable(const Pane &sub_pane) const {
    if (! Pane::IsSubPaneTouchable(sub_pane))
        return false;

    // Determine if sub_pane lies completely outside the clip rectangle.
    Range2f sub_rect = ToRange2f(TransformBounds(sub_pane.GetBounds(),
                                                 sub_pane.GetModelMatrix()));
    const Vector2f offset = GetContentsOffset();
    sub_rect.Set(sub_rect.GetMinPoint() + offset,
                 sub_rect.GetMaxPoint() + offset);
    return sub_rect.IntersectsRange(ToRange2f(GetBounds()));
}

Vector2f ClipPane::ComputeBaseSize() const {
    // Do not inherit from BoxPane; just use the minimum size. This ensures
    // that the size of the ClipPane is not affected by its contents.
    return GetMinSize();
}

Bounds ClipPane::UpdateBounds() const {
    return clip_node_->GetBounds();
}

SG::Node & ClipPane::GetContentsNode_() const {
    if (! pane_parent_)
        pane_parent_ = SG::FindNodeUnderNode(*this, "PaneParent");
    return *pane_parent_;
}
