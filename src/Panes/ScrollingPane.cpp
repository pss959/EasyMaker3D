#include "Panes/ScrollingPane.h"

void ScrollingPane::AddFields() {
    // XXXX AddField(XXXX);
    BoxPane::AddFields();
}

void ScrollingPane::SetPanes(const std::vector<PanePtr> &new_panes) {
    ReplacePanes(new_panes);
}

Vector2f ScrollingPane::ComputeMinSize() const {
    // Do not inherit from BoxPane. Use the default Pane version.
    return Pane::ComputeMinSize();
}
