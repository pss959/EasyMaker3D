#include "Panes/PaneLayout.h"

#include <algorithm>
#include <numeric>

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

    /// Computes the size for a grid layout.
    Vector2f ComputeGridSize(const GridData &col_data,
                             const GridData &row_data);

    /// Implements linear layout horizontally or vertically.
    void LayOutLinearly(PaneOrientation dir, const Vector2f &size,
                        float z_offset);

    /// Implements grid layout.
    void LayOutGrid(const Vector2f &size,
                    const GridData &col_data, const GridData &row_data);

  private:
    const ContainerPane &container_;  ///< ContainerPane to lay out.
    const float          padding_;    ///< Padding inside rectangle.
    const float          spacing_;    ///< Spacing between Panes.

    /// Computes the amount of extra size to add for expanding Panes in the
    /// given dimension, given the total size in that dimension.
    float ComputeExtraSize_(int dim, float size);

    /// Returns a vector of the base sizes of all cells in one dimension of a
    /// grid and sets \p total to the total size (including padding and
    /// spacing) in that dimension.
    std::vector<float> ComputeGridBaseSizes_(int dim, const GridData &data,
                                             size_t other_count, float &total);

    /// Adjust the given vector of cell sizes for a grid dimension to use the
    /// given amount of extra space in that dimension.
    void AdjustGridSizes_(const GridData &data, float extra,
                          std::vector<float> &sizes);

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

Vector2f PaneLayout::Impl_::ComputeGridSize(const GridData &col_data,
                                            const GridData &row_data) {
    Vector2f base_size;
    ComputeGridBaseSizes_(0, col_data, row_data.count, base_size[0]);
    ComputeGridBaseSizes_(1, row_data, col_data.count, base_size[1]);

    const auto size = MaxComponents(container_.GetMinSize(), base_size);
    KLOG('L', "Computed size " << size << " for " << container_.GetDesc());

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

void PaneLayout::Impl_::LayOutGrid(const Vector2f &size,
                                   const GridData &col_data,
                                   const GridData &row_data) {
    KLOG('L', "Laying out " << container_.GetDesc()
         << " with " << col_data.count << " X " << row_data.count
         << " cells " << " into size " << size);

    // Compute the base sizes for all rows and columns.
    Vector2f base_size;
    auto col_sizes =
        ComputeGridBaseSizes_(0, col_data, row_data.count, base_size[0]);
    auto row_sizes =
        ComputeGridBaseSizes_(1, row_data, col_data.count, base_size[1]);

    // Adjust the sizes to account for extra space, if any.
    const Vector2f extra = size - base_size;
    AdjustGridSizes_(col_data, extra[0], col_sizes);
    AdjustGridSizes_(row_data, extra[1], row_sizes);

    // Compute positions relative to the upper-left corner of the grid. Note
    // that Y decreases downward.
    const auto &panes = container_.GetPanes();
    const Point2f grid_upper_left(padding_, size[1] - padding_);
    Point2f  upper_left = grid_upper_left;
    Vector2f cell_size(0, 0);
    for (size_t row = 0; row < row_data.count; ++row) {
        cell_size[1] = row_sizes[row];
        for (size_t col = 0; col < col_data.count; ++col) {
            cell_size[0] = col_sizes[col];
            auto &pane = *panes[row * col_data.count + col];

            // If the Pane does not resize, use its base size (centered in the
            // cell). Otherwise, use the full cell size.
            const auto &resize_flags = pane.GetResizeFlags();
            Vector2f offset(0, 0);
            Vector2f pane_size = pane.GetBaseSize();
            if (resize_flags.Has(Pane::ResizeFlag::kWidth))
                pane_size[0] = cell_size[0];
            else
                offset[0] = .5f * (cell_size[0] - pane_size[0]);
            if (resize_flags.Has(Pane::ResizeFlag::kHeight))
                pane_size[1] = cell_size[1];
            else
                offset[1] = -.5f * (cell_size[1] - pane_size[1]);

            PositionPane_(pane, pane_size, size, upper_left + offset, 0);
            pane.SetRelativePositionInParent(upper_left - grid_upper_left);
            KLOG('L', "  Abs upper left of " << pane.GetDesc()
                 << " at " << (upper_left + offset)
                 << ", rel " << (upper_left - grid_upper_left));

            upper_left[0] += cell_size[0] + col_data.spacing;

            // Do this last so that any changes caused by the above functions
            // are cleared.
            pane.SetLayoutSize(pane_size);
            KLOG('L', "  Size of " << pane.GetDesc()
                 << " set to " << pane_size);
        }
        upper_left[0] = grid_upper_left[0];
        upper_left[1] -= cell_size[1] + row_data.spacing;
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

std::vector<float> PaneLayout::Impl_::ComputeGridBaseSizes_(
    int dim, const GridData &data, size_t other_count, float &total) {
    // For a column (dim = 0), the base size is the largest width of any cell
    // in that column. For a row, it is the largest height of any cell in that
    // row. This iterates over the other dimension to compute the maximum. Note
    // that this loop does NOT take padding and spacing into account.
    const size_t count     = data.count;
    const size_t col_count = dim == 0 ? count : other_count;
    std::vector<float> base_sizes(count);
    const auto &panes = container_.GetPanes();
    for (size_t i = 0; i < count; ++i) {
        base_sizes[i] = 0;
        for (size_t j = 0; j < other_count; ++j) {
            const size_t cell_index =
                dim == 0 ? j * col_count + i : i * col_count + j;
            base_sizes[i] = std::max(base_sizes[i],
                                     panes[cell_index]->GetBaseSize()[dim]);
        }
    }

    // Compute the total base size in this dimension as the sum of the
    // individual base cell sizes plus padding and spacing.
    total = 2 * padding_ + (count - 1) * data.spacing +
        std::accumulate(base_sizes.begin(), base_sizes.end(), 0.f);

    return base_sizes;
}

void PaneLayout::Impl_::AdjustGridSizes_(const GridData &data, float extra,
                                         std::vector<float> &sizes) {
    // Count the number of expanding cells in this dimension.
    const size_t expand_count = std::count(data.expands.begin(),
                                           data.expands.end(), true);

    // If nothing to expand or no space, done.
    if (extra > 0 && expand_count) {
        // Add extra to the cells that are supposed to expand.
        const float extra_per_cell = extra / expand_count;
        for (size_t i = 0; i < data.count; ++i) {
            if (data.expands[i])
                sizes[i] += extra_per_cell;
        }
    }
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

Vector2f PaneLayout::ComputeGridSize(const ContainerPane &container,
                                     float padding,
                                     const GridData &col_data,
                                     const GridData &row_data) {
    return Impl_(container, padding, 0).ComputeGridSize(col_data, row_data);
}

void PaneLayout::LayOutLinearly(const ContainerPane &container,
                                PaneOrientation dir,
                                const Vector2f &size, float padding,
                                float spacing, bool offset_forward) {
    const float z_offset = offset_forward ? TK::kPaneZOffset : 0;
    Impl_(container, padding, spacing).LayOutLinearly(dir, size, z_offset);
}

void PaneLayout::LayOutGrid(const ContainerPane &container,
                            const Vector2f &size, float padding,
                            const GridData &col_data,
                            const GridData &row_data) {
    Impl_(container, padding, 0).LayOutGrid(size, col_data, row_data);
}
