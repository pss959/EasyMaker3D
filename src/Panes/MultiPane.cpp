#include "Panes/MultiPane.h"

#include "Math/Linear.h"

void MultiPane::AddFields() {
    AddField(panes_);
    Pane::AddFields();
}

void MultiPane::PreSetUpIon() {
    Pane::PreSetUpIon();

    // Add panes as children.
    for (auto &pane: GetPanes())
        AddChild(pane);
}

void MultiPane::SetSubPaneRect(Pane &pane, const Point2f &upper_left,
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
