#include "Panels/HelpPanel.h"

#include "Util/URL.h"

void HelpPanel::InitInterface() {
    AddButtonFunc("UserGuide",  [this](){ OpenUserGuide_();  });
    AddButtonFunc("CheatSheet", [this](){ OpenCheatSheet_(); });
    AddButtonFunc("Done",       [this](){ Close_();          });
}

void HelpPanel::UpdateInterface() {
    SetFocus("Done");
}

void HelpPanel::OpenUserGuide_() {
    // XXXX Need to get version...
    Util::OpenURL("https://pss959.github.io/MakerVR-dist/0.2.1/UserGuide/");
}

void HelpPanel::OpenCheatSheet_() {
    // XXXX Need to get version...
    Util::OpenURL("https://pss959.github.io/MakerVR-dist/0.2.1/CheatSheet/");
}

void HelpPanel::Close_() {
    Close(CloseReason::kDone, "Done");
}
