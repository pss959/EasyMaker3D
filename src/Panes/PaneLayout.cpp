#include "Panes/PaneLayout.h"

#include "Math/Linear.h"
#include "Panes/ContainerPane.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/KLog.h"
#include "Util/Tuning.h"

// ----------------------------------------------------------------------------
// PaneLayout::Impl_ class that does most of the work.
// ----------------------------------------------------------------------------

class PaneLayout::Impl_ {
  public:
    /// The constructor stores all the value for easy access.
    Impl_(const ContainerPane &container, float padding, float spacing);

    /// Computes the size for a linear horizontal or vertical layout.
    Vector2f ComputeLinearSize(PaneOrientation dir);

    /// Implements linear layout horizontally or vertically.
    void LayOutLinearly(PaneOrientation dir, const Vector2f &size,
                        float z_offset);

  private:
    const ContainerPane &container_;  ///< ContainerPane to lay out.
    const float          padding_;    ///< Padding inside rectangle.
    const float          spacing_;    ///< Spacing between Panes.

    /// Computes the amount of extra size to add for expanding Panes in the
    /// given dimension, given the total size in that dimension.
    float ComputeExtraSize_(int dim, float size);

    /// Sets the scale and translation of a Pane so that it has the correct
    /// size and position relative to the given parent size, based on the given
    /// new size and upper-left corner position.
    void PositionPane_(Pane &pane, const Vector2f &pane_size,
                       const Vector2f &parent_size, const Point2f &upper_left,
                       float z_offset);

    /// Returns true if the given Pane expands in the given dimension.
    static bool PaneExpands_(const Pane &pane, int dim);

    /// Returns the dimension for the PaneOrientation.
    static int GetDim(PaneOrientation dir) {
        const int dim = Util::EnumInt(dir);
        ASSERT(dim == 0 || dim == 1);  // Keeps compiler happy.
        return dim;
    }
};

PaneLayout::Impl_::Impl_(const ContainerPane &container,
                         float padding, float spacing) :
    container_(container), padding_(padding), spacing_(spacing) {
}

Vector2f PaneLayout::Impl_::ComputeLinearSize(PaneOrientation dir) {
    // Get the base sizes of all enabled Panes.
    const auto &panes = container_.GetPanes();
    std::vector<Vector2f> base_sizes;
    base_sizes.reserve(panes.size());
    for (const auto &pane: panes)
        if (pane->IsEnabled())
            base_sizes.push_back(pane->GetBaseSize());

    // Sum the base size in both dimensions.
    const int dim        = GetDim(dir);
    const int other_dim  = 1 - dim;
    ASSERT(dim == 0 || dim == 1);  // Keeps compiler happy.

    Vector2f size(0, 0);
    for (auto &s: base_sizes) {
        size[dim]        += s[dim];
        size[other_dim]   = std::max(size[other_dim], s[other_dim]);
    }

    // Add spacing and padding.
    if (! base_sizes.empty())
        size[dim] += (base_sizes.size() - 1) * spacing_;
    size += 2 * Vector2f(padding_, padding_);

    KLOG('L', "Computed size " << size << " for " << container_.GetDesc()
         << " with direction " << Util::EnumName(dir));

    return size;
}

void PaneLayout::Impl_::LayOutLinearly(PaneOrientation dir,
                                       const Vector2f &size, float z_offset) {
    const auto &panes = container_.GetPanes();
    if (panes.empty())
        return;

    KLOG('L', "Laying out " << container_.GetDesc()
         << " with direction " << Util::EnumName(dir) << " into size " << size);

    const int dim = GetDim(dir);

    // Compute the extra size to use for resizable Panes.
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
        if (PaneExpands_(*pane, dim))
            pane_size[dim] += extra;
        pane_size[other_dim] = PaneExpands_(*pane, other_dim) ?
            size[other_dim] - 2 * padding_ : base_pane_size[other_dim];

        // Guard against rounding errors and adjust.
        pane_size = MaxComponents(pane->GetMinSize(),
                                  MaxComponents(base_pane_size, pane_size));

        PositionPane_(*pane, pane_size, size, upper_left, z_offset);

        pane->SetRelativePositionInParent(upper_left - box_upper_left);
        KLOG('L', "  Abs upper left of " << pane->GetDesc() << " at "
             << upper_left << ", rel " << (upper_left - box_upper_left));

        upper_left[dim] += sign * (pane_size[dim] + spacing_);

        // Do this last so that any changes caused by the above functions are
        // cleared.
        pane->SetLayoutSize(pane_size);
        KLOG('L', "  Size of " << pane->GetDesc() << " set to " << pane_size);
    }
}

float PaneLayout::Impl_::ComputeExtraSize_(int dim, float size) {
    // Sum the base sizes of the Panes in this dimension. While doing so, count
    // the number of Panes that resize in this dimension.
    size_t active_count = 0;
    size_t resize_count = 0;
    float  sum          = 0;
    for (const auto &pane: container_.GetPanes()) {
        if (pane->IsEnabled()) {
            ++active_count;
            sum += pane->GetBaseSize()[dim];
            if (PaneExpands_(*pane, dim))
                ++resize_count;
        }
    }
    if (resize_count == 0)
        return 0;

    const float spacing = (active_count - 1) * spacing_;
    const float padding = 2 * padding_;
    return (size - (sum + spacing + padding)) / resize_count;
}

void PaneLayout::Impl_::PositionPane_(Pane &pane, const Vector2f &pane_size,
                                      const Vector2f &parent_size,
                                      const Point2f &upper_left,
                                      float z_offset) {
    // Compute the relative size as a fraction.
    const Vector2f rel_size = pane_size / parent_size;

    // Compute the offset of the Pane's center from its upper-left corner and
    // use that to compute the relative position of the Pane's center.
    const Vector2f center_offset = Vector2f(.5f, -.5f) * pane_size;
    const Point2f  rel_center    =
        (upper_left + center_offset) / Point2f(parent_size);

    // Update the scale and translation of the Pane. Offset in Z if requested.
    pane.SetScale(Vector3f(rel_size, 1));
    pane.SetTranslation(
        Vector3f(rel_center - Point2f(.5f, .5f),
                 pane.GetTranslation()[2] + z_offset));
}

bool PaneLayout::Impl_::PaneExpands_(const Pane &pane, int dim) {
    return pane.GetResizeFlags().Has(
        dim == 0 ? Pane::ResizeFlag::kWidth : Pane::ResizeFlag::kHeight);
}

// ----------------------------------------------------------------------------
// PaneLayout functions.
// ----------------------------------------------------------------------------

Vector2f PaneLayout::ComputeLinearSize(const ContainerPane &container,
                                       PaneOrientation dir,
                                       float padding, float spacing) {
    return Impl_(container, padding, spacing).ComputeLinearSize(dir);
}

void PaneLayout::LayOutLinearly(const ContainerPane &container,
                                PaneOrientation dir,
                                const Vector2f &size, float padding,
                                float spacing, bool offset_forward) {
    const float z_offset = offset_forward ? TK::kPaneZOffset : 0;
    Impl_(container, padding, spacing).LayOutLinearly(dir, size, z_offset);
}
