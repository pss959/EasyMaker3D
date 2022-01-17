#include "Panes/ClipPane.h"

#include "SG/Search.h"

void ClipPane::CreationDone(bool is_template) {
    BoxPane::CreationDone(is_template);
    if (! is_template)
        clip_node_ = SG::FindNodeUnderNode(*this, "ClipNode");
}

Vector2f ClipPane::GetUnclippedSize() const {
    return BoxPane::ComputeMinSize();
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

Vector2f ClipPane::ComputeMinSize() const {
    // Do not inherit from BoxPane. Use the default Pane version. This ensures
    // that the size of the ClipPane is not affected by its contents.
    return Pane::ComputeMinSize();
}

Bounds ClipPane::UpdateBounds() const {
    return clip_node_->GetBounds();
}

void ClipPane::ProcessSizeChange(const Pane &initiating_pane) {
    // Change to the size of the ClipPane gets passed along. Otherwise, it has
    // absolutely no effect on the size.
    if (&initiating_pane == this)
        BoxPane::ProcessSizeChange(initiating_pane);
}

SG::Node & ClipPane::GetContentsNode_() const {
    if (! pane_parent_)
        pane_parent_ = SG::FindNodeUnderNode(*this, "PaneParent");
    return *pane_parent_;
}
