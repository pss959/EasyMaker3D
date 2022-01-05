#include "Panes/ContainerPane.h"

#include "Math/Linear.h"
#include "Util/Assert.h"

void ContainerPane::AddFields() {
    AddField(panes_);
    Pane::AddFields();
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

void ContainerPane::PreSetUpIon() {
    // Do this first so that children can be accessed when necessary.
    if (! were_panes_added_as_children_)
        AddPanesAsChildren_();

    Pane::PreSetUpIon();
}

void ContainerPane::PostSetUpIon() {
    Pane::PostSetUpIon();
    ObservePanes_();
}

void ContainerPane::SetSubPaneRect(Pane &pane, const Point2f &upper_left,
                                   const Vector2f &size) {
    // Convert the size and position to relative coordinates and
    // set the scale and translation.
    const Vector2f this_size = GetSize();

    // The relative size is just the fraction of the container size.
    const Vector2f rel_size = size / this_size;

    // Compute the relative position of the pane's center.
    const Point2f center = upper_left + Vector2f(.5f * size[0], -.5f * size[1]);
    const Point2f rel_center = center / Point2f(this_size);
    const Point2f min = rel_center - .5f * rel_size;
    const Point2f max = rel_center + .5f * rel_size;
    pane.SetRectInParent(Range2f(Clamp(min, Point2f(0, 0), Point2f(1, 1)),
                                 Clamp(max, Point2f(0, 0), Point2f(1, 1))));
}

void ContainerPane::ReplacePanes(const std::vector<PanePtr> &panes) {
    UnobservePanes_();
    RemovePanesAsChildren_();

    panes_ = panes;

    AddPanesAsChildren_();
    ObservePanes_();

    ProcessSizeChange();
}

void ContainerPane::CopyContentsFrom(const Parser::Object &from, bool is_deep) {
    Pane::CopyContentsFrom(from, is_deep);

    AddPanesAsChildren_();
    ObservePanes_(); 
}

void ContainerPane::ObservePanes_() {
    // Get notified when the size of any contained Pane may have changed.
    for (auto &pane: GetPanes()) {
        pane->GetSizeChanged().AddObserver(
            this, [this](){ ProcessSizeChange(); });
    }
}

void ContainerPane::UnobservePanes_() {
    for (auto &pane: GetPanes())
        pane->GetSizeChanged().RemoveObserver(this);
}

void ContainerPane::AddPanesAsChildren_() {
#if XXXX
    auto &aux_parent = GetAuxParent();

    // Offset each pane to move it in front.
    for (auto &pane: GetPanes()) {
        pane->SetTranslation(pane->GetTranslation() + Vector3f(0, 0, .1f));
        aux_parent.AddChild(pane);
    }

    were_panes_added_as_children_ = true;
#endif
}

void ContainerPane::RemovePanesAsChildren_() {
#if XXXX
    const auto &panes = GetPanes();
    if (! panes.empty()) {
        auto &aux_parent = GetAuxParent();
        // Go in reverse order to avoid reshuffling.
        for (auto it = panes.rbegin(); it != panes.rend(); ++it)
            aux_parent.RemoveChild(*it);
    }

    were_panes_added_as_children_ = false;
#endif
}
