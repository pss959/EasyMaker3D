#include "Panes/GridPane.h"

#include <algorithm>
#include <numeric>

#include "Assert.h"
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
    MultiPane::AddFields();
}

bool GridPane::IsValid(std::string &details) {
    if (! MultiPane::IsValid(details))
        return false;

    if (row_count_ <= 0 || column_count_ <= 0) {
        details = "Non-positive row or column count";
        return false;
    }

    if (! SetUpDim_(dim_data_[0], column_count_, expanding_columns_, details) ||
        ! SetUpDim_(dim_data_[1], row_count_,    expanding_rows_,    details))
        return false;

    if (! StorePanes_(details))
        return false;

    return true;
}

bool GridPane::StorePanes_(std::string &details) {
    // Temporarily store a pointer to this for each cell pane. This makes it
    // easy to tell which panes are not supposed to be empty.
    const size_t cell_count = dim_data_[0].count * dim_data_[1].count;
    cell_panes_.resize(cell_count, this);

    // Store a null for each empty cell and count them.
    size_t empty_count = 0;
    for (int index: empty_cells_.GetValue()) {
        if (index < 0 || static_cast<size_t>(index) >= cell_count) {
            details = "Index in empty_cells out of range";
            return false;
        }
        if (cell_panes_[index]) {
            std::cerr << "XXXX index " << index << " is NULL\n";
            cell_panes_[index] = nullptr;
            ++empty_count;
        }
    }

    // Validate the count.
    const auto &panes = GetPanes();
    if (panes.size() + empty_count != cell_count) {
        details = "Sum of contained panes (" + Util::ToString(panes.size()) +
            ") and empty panes (" + Util::ToString(empty_count) +
            ") not equal to cell count (" + Util::ToString(cell_count) + ")";
        return false;
    }

    // Store pointers to the real panes in slots with non-null pointers. This
    // should overwrite all of the "this" pointers.
    size_t index = 0;
    for (auto &pane: panes) {
        while (! cell_panes_[index])
            ++index;
        cell_panes_[index++] = pane.get();
        std::cerr << "XXXX index " << index << " is " << pane->GetDesc() << "\n";
    }
    ASSERT(! Util::Contains(cell_panes_, this));

    return true;
}

bool GridPane::SetUpDim_(DimData_ &data, size_t count,
                         const Parser::VField<int> &field,
                         std::string &details) {
    data.count = count;
    data.expands.resize(count, false);
    data.expand_count = 0;

    for (int index: field.GetValue()) {
        if (index < 0 || static_cast<size_t>(index) >= count) {
            details = "Index in " + field.GetName() + " out of range";
            return false;
        }
        if (! data.expands[index]) {
            data.expands[index] = true;
            ++data.expand_count;
        }
    }
    return true;
}

void GridPane::SetSize(const Vector2f &size) {
    MultiPane::SetSize(size);
    LayOutPanes_(size);
}

void GridPane::LayOutPanes_(const Vector2f &size) {
    // Compute the sizes for all rows and columns.
    const std::vector<float> col_sizes = ComputeSizes_(0, size[0]);
    const std::vector<float> row_sizes = ComputeSizes_(1, size[1]);

    // Compute positions relative to the upper-left corner of the grid. Note
    // that translations are within -1 to 1 and that Y decreases downward.
    Vector2f upper_left(-.5f + padding_, .5f - padding_);
    Vector2f cell_size(0, 0);
    for (size_t row = 0; row < dim_data_[1].count; ++row) {
        cell_size[1] = row_sizes[row];
        for (size_t col = 0; col < dim_data_[0].count; ++col) {
            cell_size[0] = col_sizes[col];
            if (Pane *pane = cell_panes_[GetCellIndex_(row, col)]) {
                const Vector2f min_size = pane->GetMinSize();
                // Guard against rounding errors.
                const Vector2f pane_size(std::max(min_size[0], cell_size[0]),
                                         std::max(min_size[1], cell_size[1]));
                pane->SetSize(pane_size);
                pane->SetTranslation(Vector3f(upper_left, 0));
            }
            upper_left[0] += cell_size[0] + column_spacing_;
        }
        upper_left[0] = padding_;
        upper_left[1] -= cell_size[1] + row_spacing_;
    }
}

std::vector<float> GridPane::ComputeSizes_(int dim, float size) const {
    // Start with the minimum size for each row or column, computed as the
    // largest minimum size for any cell in that row or column.
    const std::vector<float> min_sizes = ComputeMinSizes_(dim);

    // If nothing to expand, done.
    const DimData_ &data = dim_data_[dim];
    if (data.expand_count == 0)
        return min_sizes;

    // Add extra to the rows/columns that are supposed to expand.
    const float min_size =
        std::accumulate(min_sizes.begin(), min_sizes.end(), 0);
    const float extra = (size - min_size) / data.expand_count;
    std::vector<float> sizes = min_sizes;
    for (size_t i = 0; i < data.count; ++i) {
        if (data.expands[i])
            sizes[i] += extra;
    }
    return sizes;
}

std::vector<float> GridPane::ComputeMinSizes_(int dim) const {
    // The minimum size in this dimension is the sum of the largest minimum
    // cell size in the other dimension.
    const size_t count       = dim_data_[dim].count;
    const size_t other_count = dim_data_[1 - dim].count;
    std::vector<float> min_sizes(count);
    for (size_t i = 0; i < count; ++i) {
        min_sizes[i] = 0;
        for (size_t j = 0; j < other_count; ++j) {
            const size_t cell_index =
                dim == 0 ? GetCellIndex_(j, i) : GetCellIndex_(i, j);
            if (Pane *pane = cell_panes_[cell_index])
                min_sizes[i] = std::max(min_sizes[i],
                                        pane->GetMinSize()[1 - dim]);
        }
    }
    return min_sizes;
}
