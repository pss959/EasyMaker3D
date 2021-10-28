#include "Panes/GridPane.h"

void GridPane::AddFields() {
    AddField(row_count_);
    AddField(column_count_);
    AddField(spacing_);
    AddField(padding_);
    AddField(expanding_rows_);
    AddField(expanding_columns_);
    AddField(empty_cells_);
    MultiPane::AddFields();
}

bool GridPane::IsValid(std::string &details) {
    if (! MultiPane::IsValid(details))
        return false;

    const int rc = row_count_;
    const int cc = column_count_;
    if (rc <= 0 || cc <= 0) {
        details = "Non-positive row or column count";
        return false;
    }

    row_expands_.resize(rc, false);
    for (int r: expanding_rows_.GetValue()) {
        if (r < 0 || r >= rc) {
            details = "Index in expanding_rows out of range";
            return false;
        }
        row_expands_[r] = true;
    }

    col_expands_.resize(cc, false);
    for (int c: expanding_columns_.GetValue()) {
        if (c < 0 || c >= cc) {
            details = "Index in expanding_columns out of range";
            return false;
        }
        col_expands_[c] = true;
    }

    // Temporarily store a pointer to this for each cell pane.
    const size_t cell_count = rc * cc;
    cell_panes_.resize(cell_count, this);
    size_t empty_count = 0;
    for (int e: empty_cells_.GetValue()) {
        if (e < 0 || static_cast<size_t>(e) >= cell_count) {
            details = "Index in empty_cells out of range";
            return false;
        }
        if (cell_panes_[e]) {
            cell_panes_[e] = nullptr;
            ++empty_count;
        }
    }

    const auto &panes = GetPanes();
    if (panes.size() + empty_count != cell_count) {
        details = "Sum of contained panes (" + Util::ToString(panes.size()) +
            ") and empty panes (" + Util::ToString(empty_count) +
            ") not equal to cell count (" + Util::ToString(cell_count) + ")";
        return false;
    }
    // Store the real panes in slots with non-null pointers. This should
    // overwrite all of the "this" pointers.
    size_t index = 0;
    for (auto &pane: panes) {
        while (! cell_panes_[index])
            ++index;
        cell_panes_[index] = pane.get();
    }

    return true;
}

void GridPane::SetSize(const Vector2f &size) {
    // XXXX
}
