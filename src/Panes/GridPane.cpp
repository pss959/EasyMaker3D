#include "Panes/GridPane.h"

void GridPane::AddFields() {
    AddField(cell_count_);
    AddField(spacing_);
    AddField(padding_);
    AddField(expanding_rows_);
    AddField(expanding_columns_);
    AddField(empty_cells_);
    MultiPane::AddFields();
}

void GridPane::SetSize(const Vector2f &size) {
    // XXXX
}
