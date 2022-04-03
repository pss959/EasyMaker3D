#include "Panes/GridPane.h"

#include <algorithm>
#include <numeric>

#include "Math/Linear.h"
#include "Util/Assert.h"
#include "Util/General.h"

void GridPane::AddFields() {
    AddField(row_count_);
    AddField(column_count_);
    AddField(row_spacing_);
    AddField(column_spacing_);
    AddField(padding_);
    AddField(expanding_rows_);
    AddField(expanding_columns_);
    AddField(empty_cells_);
    ContainerPane::AddFields();
}

bool GridPane::IsValid(std::string &details) {
    if (! ContainerPane::IsValid(details))
        return false;

    if (row_count_ <= 0 || column_count_ <= 0) {
        details = "Non-positive row or column count";
        return false;
    }

    return CheckDim_(0, details) && CheckDim_(1, details) &&
        CheckPanes_(details);
}

void GridPane::CreationDone() {
    ContainerPane::CreationDone();

    if (! IsTemplate()) {
        SetUpDim_(0);
        SetUpDim_(1);
        StorePanes_();
    }
}

std::string GridPane::ToString() const {
    return Pane::ToString() + " " + Util::ToString(row_count_) + "R x " +
        Util::ToString(column_count_) + "C";
}

Vector2f GridPane::ComputeBaseSize() const {
    Vector2f base_size;
    ComputeBaseSizes_(0, base_size[0]);
    ComputeBaseSizes_(1, base_size[1]);
    return AdjustPaneSize(*this, base_size);
}

void GridPane::LayOutSubPanes() {
    const Vector2f size = GetLayoutSize();

    // Compute the sizes for all rows and columns.
    Vector2f base_size;
    const std::vector<float> col_sizes = ComputeSizes_(0, size[0], base_size[0]);
    const std::vector<float> row_sizes = ComputeSizes_(1, size[1], base_size[1]);

    // Compute positions relative to the upper-left corner of the grid. Note
    // that Y decreases downward.
    const Point2f grid_upper_left(padding_, size[1] - padding_);
    Point2f  upper_left = grid_upper_left;
    Vector2f cell_size(0, 0);
    for (size_t row = 0; row < dim_data_[1].count; ++row) {
        cell_size[1] = row_sizes[row];
        for (size_t col = 0; col < dim_data_[0].count; ++col) {
            cell_size[0] = col_sizes[col];
            if (Pane *pane = cell_panes_[GetCellIndex_(row, col)]) {
                // Guard against rounding errors.
                const Vector2f pane_size =
                    MaxComponents(pane->GetBaseSize(), cell_size);

                pane->SetLayoutSize(pane_size);
                PositionSubPane(*pane, upper_left);
            }
            upper_left[0] += cell_size[0] + column_spacing_;
        }
        upper_left[0] = grid_upper_left[0];
        upper_left[1] -= cell_size[1] + row_spacing_;
    }
}

bool GridPane::CheckDim_(int dim, std::string &details) {
    ASSERT(dim == 0 || dim == 1);
    const size_t count = dim == 0 ? column_count_      : row_count_;
    const auto  &field = dim == 0 ? expanding_columns_ : expanding_rows_;
    for (int index: field.GetValue()) {
        if (index < 0 || static_cast<size_t>(index) >= count) {
            details = "Index in " + field.GetName() + " out of range";
            return false;
        }
    }
    return true;
}

bool GridPane::CheckPanes_(std::string &details) {
    const size_t cell_count = row_count_.GetValue() * column_count_.GetValue();

    // Check empty cells.
    for (int index: empty_cells_.GetValue()) {
        if (index < 0 || static_cast<size_t>(index) >= cell_count) {
            details = "Index in empty_cells out of range";
            return false;
        }
    }

    // Validate the count.
    const auto &panes = GetPanes();
    const size_t empty_count = empty_cells_.GetValue().size();
    if (panes.size() + empty_count != cell_count) {
        details = "Sum of contained panes (" + Util::ToString(panes.size()) +
            ") and empty panes (" + Util::ToString(empty_count) +
            ") not equal to cell count (" + Util::ToString(cell_count) + ")";
        return false;
    }

    return true;
}

void GridPane::SetUpDim_(int dim) {
    ASSERT(dim == 0 || dim == 1);
    DimData_ &data = dim_data_[dim];

    const auto &field = dim == 0 ? expanding_columns_ : expanding_rows_;
    data.count        = dim == 0 ? column_count_      : row_count_;
    data.spacing      = dim == 0 ? column_spacing_    : row_spacing_;

    data.expands.resize(data.count, false);
    data.expand_count = 0;

    for (int index: field.GetValue()) {
        if (! data.expands[index]) {
            data.expands[index] = true;
            ++data.expand_count;
        }
    }
}

void GridPane::StorePanes_() {
    // Temporarily store a pointer to this for each cell pane. This makes it
    // easy to tell which panes are not supposed to be empty.
    const size_t cell_count = dim_data_[0].count * dim_data_[1].count;
    cell_panes_.resize(cell_count, this);

    // Store a null for each empty cell and count them.
    size_t empty_count = 0;
    for (int index: empty_cells_.GetValue()) {
        if (cell_panes_[index]) {
            cell_panes_[index] = nullptr;
            ++empty_count;
        }
    }

    // Store pointers to the real panes in slots with non-null pointers. This
    // should overwrite all of the "this" pointers.
    size_t index = 0;
    for (auto &pane: GetPanes()) {
        while (! cell_panes_[index])
            ++index;
        cell_panes_[index++] = pane.get();
    }
    ASSERT(! Util::Contains(cell_panes_, this));
}

std::vector<float> GridPane::ComputeSizes_(int dim, float size,
                                           float &base_size) const {
    // Start with the base size for each row or column, computed as the largest
    // base size for any cell in that row or column.
    const std::vector<float> base_sizes = ComputeBaseSizes_(dim, base_size);

    // If nothing to expand, done.
    const DimData_ &data = dim_data_[dim];
    if (data.expand_count == 0)
        return base_sizes;

    // Add extra to the rows/columns that are supposed to expand.
    const float extra = (size - base_size) / data.expand_count;
    std::vector<float> sizes = base_sizes;
    for (size_t i = 0; i < data.count; ++i) {
        if (data.expands[i])
            sizes[i] += extra;
    }
    return sizes;
}

std::vector<float> GridPane::ComputeBaseSizes_(int dim, float &total) const {
    // For any column (dim = 0), the base size is the largest width of any cell
    // in that column. For any row, it is the largest height of any cell in
    // that row. This iterates over the other dimension to compute the
    // maximum. Note that this does NOT take padding and spacing into account.
    const size_t count       = dim_data_[dim].count;
    const size_t other_count = dim_data_[1 - dim].count;
    std::vector<float> base_sizes(count);
    for (size_t i = 0; i < count; ++i) {
        base_sizes[i] = 0;
        for (size_t j = 0; j < other_count; ++j) {
            const size_t cell_index =
                dim == 0 ? GetCellIndex_(j, i) : GetCellIndex_(i, j);
            if (Pane *pane = cell_panes_[cell_index])
                base_sizes[i] = std::max(base_sizes[i],
                                         pane->GetBaseSize()[dim]);
        }
    }

    // Compute the total base size in this dimension as the sum of the
    // individual base cell sizes plus padding and spacing.
    total = 2 * padding_ + (count - 1) * dim_data_[dim].spacing +
        std::accumulate(base_sizes.begin(), base_sizes.end(), 0.f);

    return base_sizes;
}
