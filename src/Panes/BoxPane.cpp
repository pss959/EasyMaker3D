#include "Panes/BoxPane.h"

#include <algorithm>
#include <vector>

#include "Math/Linear.h"
#include "Util/Assert.h"
#include "Util/General.h"

void BoxPane::AddFields() {
    AddField(orientation_);
    AddField(spacing_);
    AddField(padding_);
    ContainerPane::AddFields();
}

std::string BoxPane::ToString() const {
    return Pane::ToString() +
        (GetOrientation() == Orientation::kVertical ? " [V]" : " [H]");
}

Vector2f BoxPane::ComputeBaseSize() const {
    // Get the base sizes of all enabled contained Panes.
    const auto &panes = GetPanes();
    std::vector<Vector2f> base_sizes;
    base_sizes.reserve(panes.size());
    for (const auto &pane: panes)
        if (pane->IsEnabled())
            base_sizes.push_back(pane->GetBaseSize());

    // Sum the base size in both dimensions.
    const int layout_dim = GetOrientation() == Orientation::kVertical ? 1 : 0;
    const int other_dim  = 1 - layout_dim;

    Vector2f base_size(0, 0);
    for (auto &s: base_sizes) {
        base_size[layout_dim] += s[layout_dim];
        base_size[other_dim]   = std::max(base_size[other_dim], s[other_dim]);
    }

    // Add spacing and padding.
    if (! base_sizes.empty())
        base_size[layout_dim] += (base_sizes.size() - 1) * spacing_;
    base_size += 2 * Vector2f(padding_, padding_);

    return ClampSize(*this, base_size);
}

void BoxPane::LayOutPanes(const Vector2f &size) {
    const auto &panes = GetPanes();
    if (panes.empty())
        return;

    // Compute the extra size to use for resizable Panes.
    const int dim  = GetOrientation() == Orientation::kVertical ? 1 : 0;
    const float extra = ComputeExtraSize_(dim, size[dim]);

    // Compute positions relative to the upper-left corner of the box. Note
    // that Y decreases downward.
    const int sign = dim == 0 ? 1 : -1;
    const Point2f box_upper_left(padding_, size[1] - padding_);
    Point2f upper_left = box_upper_left;
    const int other_dim  = 1 - dim;
    for (const auto &pane: panes) {
        if (! pane->IsEnabled())
            continue;

        const Vector2f base_pane_size = pane->GetBaseSize();
        Vector2f pane_size;
        pane_size[dim] = base_pane_size[dim];
        if (Expands_(*pane, dim))
            pane_size[dim] += extra;
        pane_size[other_dim] = Expands_(*pane, other_dim) ?
            size[other_dim] - 2 * padding_ : base_pane_size[other_dim];

        // Guard against rounding errors and clamp.
        pane_size = ClampSize(*pane, MaxComponents(base_pane_size, pane_size));

        pane->SetSizeWithinContainer(
            pane_size, ComputeSubPaneRect(size, pane_size, upper_left));

        upper_left[dim] += sign * (pane_size[dim] + spacing_);
    }
}

float BoxPane::ComputeExtraSize_(int dim, float size) {
    // Sum the base sizes of the Panes in this dimension. While doing so, count
    // the number of Panes that resize in this dimension.
    const auto &panes = GetPanes();
    size_t active_count = 0;
    size_t resize_count = 0;
    float  sum          = 0;
    for (const auto &pane: panes) {
        if (pane->IsEnabled()) {
            ++active_count;
            sum += pane->GetBaseSize()[dim];
            if (Expands_(*pane, dim))
                ++resize_count;
        }
    }
    if (resize_count == 0)
        return 0;

    const float spacing = (active_count - 1) * spacing_;
    const float padding = 2 * padding_;
    return (size - (sum + spacing + padding)) / resize_count;
}
