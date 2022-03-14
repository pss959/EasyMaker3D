#include "Panels/ActionPanel.h"

void ActionPanel::InitInterface() {
    AddButtonFunc("Cancel", [&](){ Close("Cancel"); });
    AddButtonFunc("Accept", [&](){ Accept_(); });
}

void ActionPanel::UpdateInterface() {
    SetFocus("Cancel");
}

void ActionPanel::Accept_() {
    // XXXX Do something.
}
