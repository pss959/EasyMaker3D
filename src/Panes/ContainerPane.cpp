#include "Panes/ContainerPane.h"

#include "Math/Linear.h"
#include "Util/Assert.h"
#include "Util/KLog.h"

ContainerPane::~ContainerPane() {
    if (were_panes_observed_)
        UnobservePanes_();
}

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

void ContainerPane::PostSetUpIon() {
    Pane::PostSetUpIon();
    if (! were_panes_observed_)  // Could be true if cloned.
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
    panes_ = panes;
    ObservePanes_();
    ProcessSizeChange();
}

void ContainerPane::CopyContentsFrom(const Parser::Object &from, bool is_deep) {
    Pane::CopyContentsFrom(from, is_deep);
    ObservePanes_();
}

void ContainerPane::AddExtraChildren(std::vector<SG::NodePtr> &children) const {
    auto caster = [](const PanePtr &p){ return Util::CastToBase<SG::Node>(p); };
    Util::AppendVector(
        Util::ConvertVector<SG::NodePtr, PanePtr>(GetPanes(), caster),
        children);
}

void ContainerPane::ObservePanes_() {
    ASSERT(! were_panes_observed_);
    // Get notified when the size of any contained Pane may have changed.
    for (auto &pane: GetPanes()) {
        KLOG('o', GetDesc() + " observing " + pane->GetDesc());
        pane->GetSizeChanged().AddObserver(
            this, [this](){ ProcessSizeChange(); });
    }
    were_panes_observed_ = true;
}

void ContainerPane::UnobservePanes_() {
    ASSERT(were_panes_observed_);
    for (auto &pane: GetPanes()) {
        KLOG('o', GetDesc() + " unobserving  " + pane->GetDesc());
        pane->GetSizeChanged().RemoveObserver(this);
    }
    were_panes_observed_ = false;
}
