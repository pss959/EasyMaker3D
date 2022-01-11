#include "Panes/ClipPane.h"

#include "SG/Search.h"

void ClipPane::AllFieldsParsed(bool is_template) {
    BoxPane::AllFieldsParsed(is_template);
    pane_parent_ = SG::FindNodeUnderNode(*this, "PaneParent");
    clip_node_   = SG::FindNodeUnderNode(*this, "ClipNode");
}

Vector2f ClipPane::ComputeMinSize() const {
    // Do not inherit from BoxPane. Use the default Pane version. This ensures
    // that the size of the ClipPane is not affected by its contents.
    return Pane::ComputeMinSize();
}
