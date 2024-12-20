//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panes/SwitcherPane.h"

#include "Math/Linear.h"
#include "Util/String.h"

void SwitcherPane::AddFields() {
    AddField(index_.Init("index", -1));

    ContainerPane::AddFields();
}

void SwitcherPane::CreationDone() {
    ContainerPane::CreationDone();

    // Force update.
    UpdateIndex_(index_, true);
}

void SwitcherPane::SetIndex(int index) {
    UpdateIndex_(index, false);
}

// LCOV_EXCL_START [debug only]
Str SwitcherPane::ToString(bool is_brief) const {
    // Add the current index
    return Pane::ToString(is_brief) + " IX=" + Util::ToString(GetIndex());
}
// LCOV_EXCL_STOP

Vector2f SwitcherPane::ComputeBaseSize() const {
    // Start with the minimum size of this Pane.
    Vector2f base_size = GetMinSize();

    // Get the base sizes of all contained panes and use the maximum.
    for (const auto &pane: GetPanes())
        base_size = MaxComponents(base_size, pane->GetBaseSize());

    return AdjustPaneSize(*this, base_size);
}

void SwitcherPane::LayOutSubPanes() {
    const int index = GetIndex();
    const auto &panes = GetPanes();
    if (panes.empty() || index < 0)
        return;

    // Lay out the one visible pane to fill this Pane's size. Offset it forward
    // if this Pane has a background.
    auto &pane = *panes[index];
    const auto size = GetLayoutSize();
    pane.SetLayoutSize(size);
    PositionSubPane(pane, Point2f(0, size[1]), HasBackground());
}

void SwitcherPane::UpdateIndex_(int new_index, bool force_update) {
    const int index_to_use =
        new_index < 0 || static_cast<size_t>(new_index) >= GetPanes().size() ?
        -1 : new_index;

    // Avoid notifying if not really changing the index.
    if (force_update || GetIndex() != index_to_use) {
        index_ = index_to_use;

        // Enable or disable all sub-panes.
        const auto &panes = GetPanes();
        for (int i = 0; i < static_cast<int>(panes.size()); ++i)
            panes[i]->SetEnabled(i == index_to_use);

        MarkLayoutAsChanged();
    }
}
