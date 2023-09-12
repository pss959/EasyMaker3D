#include "Panes/BoxPane.h"

#include <algorithm>
#include <vector>

#include "Math/Linear.h"
#include "Panes/PaneLayout.h"

void BoxPane::AddFields() {
    AddField(orientation_.Init("orientation", PaneOrientation::kVertical));
    AddField(spacing_.Init("spacing", 0));
    AddField(padding_.Init("padding", 0));

    ContainerPane::AddFields();
}

// LCOV_EXCL_START [debug only]
Str BoxPane::ToString(bool is_brief) const {
    return Pane::ToString(is_brief) +
        (GetOrientation() == PaneOrientation::kVertical ? " [V]" : " [H]");
}
// LCOV_EXCL_STOP

Vector2f BoxPane::ComputeBaseSize() const {
    const auto size = PaneLayout::ComputeLinearSize(*this, GetOrientation(),
                                                    padding_, spacing_);
    return MaxComponents(GetMinSize(), size);
}

void BoxPane::LayOutSubPanes() {
    PaneLayout::LayOutLinearly(*this, GetOrientation(), GetLayoutSize(),
                               padding_, spacing_, false);
}
