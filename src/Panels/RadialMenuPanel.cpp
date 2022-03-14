#include "Panels/RadialMenuPanel.h"

#include "Settings.h"

void RadialMenuPanel::InitInterface() {
    AddButtonFunc("Cancel", [&](){ Close("Cancel"); });
    AddButtonFunc("Accept", [&](){ AcceptEdits_(); });
}

void RadialMenuPanel::UpdateInterface() {
    // XXXX
}

void RadialMenuPanel::AcceptEdits_() {
    // XXXX
}
