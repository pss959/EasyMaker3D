#include "Panes/ContainerPane.h"

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
        if (! panes.empty())
            SizeChanged(*this);
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

    // Force derived class to lay out panes again.
    const Vector2f size = GetSize();
    if (size != Vector2f::Zero())
        SetSize(size);
}

void ContainerPane::SetSize(const Vector2f &size) {
    // Lay panes out first so that size changes do not notify as much.
    LayOutPanes(size);
    Pane::SetSize(size);
}

void ContainerPane::OffsetPanes_() {
    for (auto &pane: GetPanes())
        pane->SetTranslation(pane->GetTranslation() + Vector3f(0, 0, .1f));
}

void ContainerPane::SetSubPaneRect(Pane &sub_pane,
                                   const Vector2f &container_pane_size,
                                   const Vector2f &sub_pane_size,
                                   const Point2f &upper_left) {
    // Compute the relative size as a fraction.
    const Vector2f rel_size = sub_pane_size / container_pane_size;

    // Compute the offset of the sub Pane's center from its upper-left corner.
    const Vector2f center_offset = Vector2f(.5f, -.5f) * sub_pane_size;

    // Compute the relative position of the Pane's center.
    const Point2f rel_center =
        (upper_left + center_offset) / Point2f(container_pane_size);

    // Update the scale and translation in the sub Pane based on the rectangle.
    const Vector2f trans = rel_center - Point2f(.5f, .5f);
    sub_pane.SetScale(Vector3f(rel_size, 1));
    sub_pane.SetTranslation(Vector3f(trans, sub_pane.GetTranslation()[2]));
}

void ContainerPane::ObservePanes_() {
    // Get notified when the size of any contained Pane may have changed.
    for (auto &pane: GetPanes()) {
        KLOG('o', GetDesc() + " observing " + pane->GetDesc());
        pane->GetSizeChanged().AddObserver(this, [&](const Pane &p){
            SizeChanged(p);
        });
    }
}

void ContainerPane::UnobservePanes_() {
    for (auto &pane: GetPanes()) {
        KLOG('o', GetDesc() + " unobserving  " + pane->GetDesc());
        pane->GetSizeChanged().RemoveObserver(this);
    }
}
