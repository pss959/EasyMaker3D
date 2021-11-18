#include "Panels/HelpPanel.h"

void HelpPanel::InitInterface() {
    AddButtonFunc("UserGuide",  [this](){ OpenUserGuide_();  });
    AddButtonFunc("CheatSheet", [this](){ OpenCheatSheet_(); });
    AddButtonFunc("Done",       [this](){ Close_();          });
}

void HelpPanel::UpdateInterface() {
    SetFocus("Done");
}

void HelpPanel::OpenUserGuide_() {
    // XXXX TEMPORARY
    Close(CloseReason::kDone, "Done");
}

void HelpPanel::OpenCheatSheet_() {
    // XXXX TEMPORARY
    Close(CloseReason::kDone, "Done");
}

void HelpPanel::Close_() {
    Close(CloseReason::kDone, "Done");
}
