#include "Panes/MultiPane.h"

void MultiPane::AddFields() {
    AddField(panes_);
    Pane::AddFields();
}
