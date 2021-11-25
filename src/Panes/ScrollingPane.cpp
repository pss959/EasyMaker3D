#include "Panes/ScrollingPane.h"

void ScrollingPane::AddFields() {
    // XXXX AddField(XXXX);
    BoxPane::AddFields();
}

void ScrollingPane::SetPanes(const std::vector<PanePtr> &new_panes) {
    ReplacePanes(new_panes);
}
