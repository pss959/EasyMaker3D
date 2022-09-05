#include "Panes/ClipPane.h"

#include "Math/Linear.h"
#include "SG/Search.h"

void ClipPane::CreationDone() {
    BoxPane::CreationDone();

    if (! IsTemplate())
        clip_node_ = SG::FindNodeUnderNode(*this, "ClipNode");
}

Vector2f ClipPane::GetUnclippedSize() {
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

WidgetPtr ClipPane::GetIntersectedWidget(const IntersectionFunc &func,
                                         float &closest_distance) {
    // Let the base Pane class test this Pane.
    WidgetPtr best_widget = Pane::GetIntersectedWidget(func, closest_distance);

    // Try unclipped contained Panes as well. This is the same as in the
    // ContainerPane version, except that this skips contained Panes that do
    // not overlap the clip area. Do all the math in 2D because the Z
    // coordinates may differ.
    const Range2f &clip_rect = ToRange2f(GetBounds());
    const Vector2f offset = GetContentsOffset();
    for (auto &pane: GetPanes()) {
        if (! pane->IsEnabled())
            continue;
        Range2f pane_rect = ToRange2f(TransformBounds(pane->GetBounds(),
                                                      pane->GetModelMatrix()));
        pane_rect.Set(pane_rect.GetMinPoint() + offset,
                      pane_rect.GetMaxPoint() + offset);
        if (pane_rect.IntersectsRange(clip_rect)) {
            if (WidgetPtr widget =
                pane->GetIntersectedWidget(func, closest_distance))
                best_widget = widget;
        }
    }
    return best_widget;
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
