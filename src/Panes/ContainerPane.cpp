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
        for (const auto &pane: GetPanes())
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
    Pane::SetSize(size);
    LayOutPanes(size);
}

void ContainerPane::OffsetPanes_() {
    for (auto &pane: GetPanes())
        pane->SetTranslation(pane->GetTranslation() + Vector3f(0, 0, .1f));
}

void ContainerPane::SetSubPaneRect(Pane &pane, const Point2f &upper_left,
                                   const Vector2f &size) {
    // Compute the relative size as the fraction of the container size and the
    // relative position of the Pane's center. Allow these to be outside the
    // 0-1 range, since Panes in a ClipPane may be outside the clip rectangle.
    const Vector2f &this_size = GetSize();
    const Vector2f rel_size   = size / this_size;
    const Vector2f center_offset = Vector2f(.5f, -.5f) * size;
    const Point2f  rel_center =
        (upper_left + center_offset) / Point2f(this_size);

    // Update the scale and translation in the Pane based on the rectangle.
    SetScale(Vector3f(rel_size, 1));
    SetTranslation(Vector3f(rel_center - Point2f(.5f, .5f),
                            GetTranslation()[2]));
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
