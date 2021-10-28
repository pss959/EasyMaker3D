#include "Panes/MultiPane.h"

void MultiPane::AddFields() {
    AddField(panes_);
    Pane::AddFields();
}

void MultiPane::PreSetUpIon() {
    // Add panes as children before letting the base class set up.
    for (auto &pane: GetPanes())
        AddChild(pane);
}
