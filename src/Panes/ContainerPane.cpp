#include "Panes/ContainerPane.h"

#include "Math/Linear.h"
#include "Util/Assert.h"
#include "Util/KLog.h"

ContainerPane::~ContainerPane() {
    if (IsCreationDone() && ! IsTemplate())
        UnobservePanes_();
}

void ContainerPane::AddFields() {
    AddField(panes_);
    Pane::AddFields();
}

void ContainerPane::CreationDone() {
    Pane::CreationDone();

    if (! IsTemplate()) {
        OffsetPanes_();
        ObservePanes_();

        // Add all contained panes as extra children.
        SG::Node &parent = GetExtraChildParent();
        parent.ClearExtraChildren();
        const auto &panes = GetPanes();
        for (const auto &pane: panes)
            parent.AddExtraChild(pane);
    }
}

PanePtr ContainerPane::FindPane(const std::string &name) const {
    // Note that this cannot check "this" because it has to return a shared_ptr.
    for (const auto &pane: GetPanes()) {
        if (pane->GetName() == name)
            return pane;
        // Recurse if this is a ContainerPane.
        if (ContainerPanePtr ctr = Util::CastToDerived<ContainerPane>(pane)) {
            if (PanePtr found = ctr->FindPane(name))
                return found;
        }
    }
    return PanePtr();
}

void ContainerPane::RemovePane(const PanePtr &pane) {
    const auto &panes = GetPanes();
    size_t index = panes.size();
    for (size_t i = 0; i < panes.size(); ++i) {
        if (panes[i] == pane) {
            index = i;
            break;
        }
    }
    ASSERT(index < panes.size());
    panes_.Remove(index);

    // Notify that size and contents may have changed.
    BaseSizeChanged();
    ContentsChanged();
}

void ContainerPane::ReplacePanes(const std::vector<PanePtr> &panes) {
    ASSERT(IsCreationDone());

    SG::Node &parent = GetExtraChildParent();
    parent.ClearExtraChildren();
    UnobservePanes_();
    panes_ = panes;
    OffsetPanes_();
    ObservePanes_();
    for (const auto &pane: GetPanes())
        parent.AddExtraChild(pane);

    // Notify that size and contents may have changed.
    BaseSizeChanged();
    ContentsChanged();
}

void ContainerPane::SetLayoutSize(const Vector2f &size) {
    if (size != GetLayoutSize()) {
        Pane::SetLayoutSize(size);
        KLOG('p', "Laying out Panes for " << GetDesc()
             << " with size " << size);

        // Let the derived class update the layout size in contained Panes.
        LayOutSubPanes();
    }
}

void ContainerPane::OffsetPanes_() {
    const float z_offset = HasBackground() ? .2f : .1f;
    for (auto &pane: GetPanes())
        if (pane->IsEnabled())
            pane->SetTranslation(pane->GetTranslation() +
                                 Vector3f(0, 0, z_offset));
}

void ContainerPane::PositionSubPane(Pane &sub_pane, const Point2f &upper_left,
                                    bool offset_forward) {
    const Vector2f &ctr_pane_size = GetLayoutSize();
    const Vector2f &sub_pane_size = sub_pane.GetLayoutSize();

    // Compute the relative size as a fraction.
    const Vector2f rel_size = sub_pane_size / ctr_pane_size;

    // Compute the offset of the sub Pane's center from its upper-left corner.
    const Vector2f center_offset = Vector2f(.5f, -.5f) * sub_pane_size;

    // Compute the relative position of the sub Pane's center.
    const Point2f rel_center =
        (upper_left + center_offset) / Point2f(ctr_pane_size);

    // Update the scale and translation of the sub Pane.
    sub_pane.SetScale(Vector3f(rel_size, 1));
    Vector3f trans(rel_center - Point2f(.5f, .5f),
                   sub_pane.GetTranslation()[2]);
    if (offset_forward)
        trans[2] += .1f;
    sub_pane.SetTranslation(trans);
}

Vector2f ContainerPane::AdjustPaneSize(const Pane &pane, const Vector2f &size) {
    const Vector2f &min = pane.GetMinSize();
    return Vector2f(std::max(min[0], size[0]), std::max(min[1], size[1]));
}

void ContainerPane::ObservePanes_() {
    // Get notified when the base size or contents of any contained Pane may
    // have changed.
    for (auto &pane: GetPanes()) {
        KLOG('o', GetDesc() + " observing " + pane->GetDesc());
        pane->GetBaseSizeChanged().AddObserver(
            this, [&](){ BaseSizeChanged(); });
        if (ContainerPanePtr ctr = Util::CastToDerived<ContainerPane>(pane))
            ctr->GetContentsChanged().AddObserver(
                this, [&](){ ContentsChanged(); });
    }
}

void ContainerPane::UnobservePanes_() {
    for (auto &pane: GetPanes()) {
        KLOG('o', GetDesc() + " unobserving  " + pane->GetDesc());
        pane->GetBaseSizeChanged().RemoveObserver(this);
        if (ContainerPanePtr ctr = Util::CastToDerived<ContainerPane>(pane))
            ctr->GetContentsChanged().RemoveObserver(this);
    }
}
