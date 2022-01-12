#include "Panes/ClipPane.h"

#include "SG/Search.h"

void ClipPane::AllFieldsParsed(bool is_template) {
    BoxPane::AllFieldsParsed(is_template);
    pane_parent_ = SG::FindNodeUnderNode(*this, "PaneParent");
    clip_node_   = SG::FindNodeUnderNode(*this, "ClipNode");
}

Vector2f ClipPane::GetUnclippedSize() const {
    return BoxPane::ComputeMinSize();
}

void ClipPane::SetContentsOffset(const Vector2f &offset) {
    const Vector3f trans = pane_parent_->GetTranslation();
    pane_parent_->SetTranslation(Vector3f(offset[0], offset[1], trans[2]));
}

Vector2f ClipPane::GetContentsOffset() const {
    const Vector3f &trans = pane_parent_->GetTranslation();
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
