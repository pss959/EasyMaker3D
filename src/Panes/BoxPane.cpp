#include "Panes/BoxPane.h"

#include <algorithm>
#include <vector>

#include "Math/Linear.h"
#include "Panes/PaneLayout.h"

void BoxPane::AddFields() {
    AddField(orientation_.Init("orientation", Orientation::kVertical));
    AddField(spacing_.Init("spacing", 0));
    AddField(padding_.Init("padding", 0));

    ContainerPane::AddFields();
}

// LCOV_EXCL_START [debug only]
Str BoxPane::ToString(bool is_brief) const {
    return Pane::ToString(is_brief) +
        (GetOrientation() == Orientation::kVertical ? " [V]" : " [H]");
}
// LCOV_EXCL_STOP

Vector2f BoxPane::ComputeBaseSize() const {
    const auto size = GetOrientation() == Orientation::kVertical ?
        PaneLayout::ComputeVerticalSize(GetPanes(), padding_, spacing_) :
        PaneLayout::ComputeHorizontalSize(GetPanes(), padding_, spacing_);
    return MaxComponents(GetMinSize(), size);
}

void BoxPane::LayOutSubPanes() {
    if (GetOrientation() == Orientation::kVertical)
        PaneLayout::LayOutVertically(GetPanes(), GetLayoutSize(), padding_,
                                     spacing_, false);
    else
        PaneLayout::LayOutHorizontally(GetPanes(), GetLayoutSize(), padding_,
                                       spacing_, false);
}
