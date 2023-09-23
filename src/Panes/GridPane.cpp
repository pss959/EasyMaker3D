#include "Panes/GridPane.h"

#include <algorithm>
#include <numeric>

#include "Math/Linear.h"
#include "Panes/PaneLayout.h"
#include "Util/Assert.h"
#include "Util/String.h"

void GridPane::AddFields() {
    AddField(row_count_.Init("row_count",           1));
    AddField(column_count_.Init("column_count",     1));
    AddField(row_spacing_.Init("row_spacing",       0));
    AddField(column_spacing_.Init("column_spacing", 0));
    AddField(padding_.Init("padding",               0));
    AddField(expanding_rows_.Init("expanding_rows"));
    AddField(expanding_columns_.Init("expanding_columns"));

    ContainerPane::AddFields();
}

bool GridPane::IsValid(Str &details) {
    if (! ContainerPane::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
    if (row_count_ <= 0 || column_count_ <= 0) {
        details = "Non-positive row or column count";
        return false;
    }
    return CheckDim_(0, details) && CheckDim_(1, details) &&
        CheckPanes_(details);
}

// LCOV_EXCL_START [debug only]
Str GridPane::ToString(bool is_brief) const {
    return Pane::ToString(is_brief) + " " +
        Util::ToString(row_count_) + "R x " +
        Util::ToString(column_count_) + "C";
}
// LCOV_EXCL_STOP

Vector2f GridPane::ComputeBaseSize() const {
    return PaneLayout::ComputeGridSize(*this, padding_,
                                       GetData_(0), GetData_(1));
}

void GridPane::LayOutSubPanes() {
    PaneLayout::LayOutGrid(*this, GetLayoutSize(), padding_,
                           GetData_(0), GetData_(1));
}

bool GridPane::CheckDim_(int dim, Str &details) {
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

bool GridPane::CheckPanes_(Str &details) {
    // Validate the Pane count.
    const size_t cell_count = row_count_.GetValue() * column_count_.GetValue();
    const auto &panes = GetPanes();
    if (panes.size() != cell_count) {
        details = "Number of contained panes (" + Util::ToString(panes.size()) +
            ") not equal to cell count (" + Util::ToString(cell_count) + ")";
        return false;
    }
    return true;
}

PaneLayout::GridData GridPane::GetData_(int dim) const {
    ASSERT(dim == 0 || dim == 1);
    PaneLayout::GridData data;

    const auto &field = dim == 0 ? expanding_columns_ : expanding_rows_;
    data.count        = dim == 0 ? column_count_      : row_count_;
    data.spacing      = dim == 0 ? column_spacing_    : row_spacing_;

    data.expands.resize(data.count, false);
    for (int index: field.GetValue())
        data.expands[index] = true;

    return data;
}
