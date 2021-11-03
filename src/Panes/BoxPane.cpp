#include "Panes/BoxPane.h"

#include <algorithm>
#include <vector>

#include "Assert.h"
#include "Math/Linear.h"
#include "Util/General.h"

void BoxPane::AddFields() {
    AddField(orientation_);
    AddField(spacing_);
    AddField(padding_);
    ContainerPane::AddFields();
}

bool BoxPane::IsValid(std::string &details) {
    if (! ContainerPane::IsValid(details))
        return false;

    return true;
}

void BoxPane::SetSize(const Vector2f &size) {
    ContainerPane::SetSize(size);
    LayOutPanes_(size);
}

void BoxPane::LayOutPanes_(const Vector2f &size) {
    const auto &panes = GetPanes();
    if (panes.empty())
        return;

    const Vector2f min_size = ComputeMinSize_();
    SetMinSize(MaxComponents(GetBaseSize(), min_size));

    // Count the number of elements that will expand and use that to compute
    // the extra size (if any) for each of them.
    const int dim  = GetOrientation() == Orientation::kVertical ? 1 : 0;
    auto expands = [dim](const PanePtr &pane){
        return dim == 0 ? pane->IsWidthResizable() : pane->IsHeightResizable();
    };
    const int resize_count = std::count_if(panes.begin(), panes.end(), expands);
    const float extra = resize_count == 0 ? 0.f :
        (size[dim] - min_size[dim]) / resize_count;

    // Compute positions relative to the upper-left corner of the grid. Note
    // that Y decreases downward.
    const int sign = dim == 0 ? 1 : -1;
    const Point2f box_upper_left(padding_, size[1] - padding_);
    Point2f  upper_left = box_upper_left;
    const float other_size = size[1 - dim] - 2 * padding_;
    for (const auto &pane: panes) {
        const Vector2f min_pane_size = pane->GetMinSize();
        Vector2f pane_size;
        pane_size[dim] = min_pane_size[dim] + (expands(pane) ? extra : 0.f);
        pane_size[1 - dim] = other_size;

        // Guard against rounding errors.
        pane_size = MaxComponents(min_pane_size, pane_size);

        // Set the world-space size of the contained Pane.
        pane->SetSize(pane_size);

        // Scale and position the pane.
        SetSubPaneRect(*pane, upper_left, pane_size);

        upper_left[dim] += sign * (pane_size[dim] + spacing_);
    }
}

Vector2f BoxPane::ComputeMinSize_() {
    // Get the minimum sizes of all contained panes.
    const auto &panes = GetPanes();
    std::vector<Vector2f> min_sizes;
    min_sizes.reserve(panes.size());
    for (const auto &pane: panes)
        min_sizes.push_back(pane->GetMinSize());

    // Compute the minimum size in both dimensions.
    Vector2f min_size(0, 0);
    if (GetOrientation() == Orientation::kVertical) {
        for (auto &s: min_sizes) {
            min_size[0] = std::max(min_size[0], s[0]);
            min_size[1] += s[1];
        }
        min_size[1] += (panes.size() - 1) * spacing_;
    }
    else {
        for (auto &s: min_sizes) {
            min_size[0] += s[0];
            min_size[1] = std::max(min_size[1], s[1]);
        }
        min_size[0] += (panes.size() - 1) * spacing_;
    }
    min_size += 2 * Vector2f(padding_, padding_);
    return min_size;
}
