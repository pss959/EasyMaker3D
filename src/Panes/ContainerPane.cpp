#include "Panes/ContainerPane.h"

#include "Math/Linear.h"

void ContainerPane::AddFields() {
    AddField(panes_);
    Pane::AddFields();
}

void ContainerPane::PreSetUpIon() {
    Pane::PreSetUpIon();

    // Add panes as children. Offset each to move it in front.
    auto &aux_parent = GetAuxParent();
    for (auto &pane: GetPanes()) {
        pane->SetTranslation(pane->GetTranslation() + Vector3f(0, 0, .1f));
        aux_parent.AddChild(pane);
    }
}

void ContainerPane::PostSetUpIon() {
    Pane::PostSetUpIon();

    // Get notified when the size of any contained Pane may have changed. Note
    // that it is ok to update the minimum size for each one, since they should
    // be fairly rare.
    for (auto &pane: GetPanes())
        pane->GetSizeChanged().AddObserver(
            this, [this](){ ProcessSizeChange(); });
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
