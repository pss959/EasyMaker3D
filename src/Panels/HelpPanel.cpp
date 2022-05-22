#include "Panels/HelpPanel.h"

#include "Util/URL.h"

void HelpPanel::InitInterface() {
    AddButtonFunc("UserGuide",  [this](){ OpenUserGuide_();  });
    AddButtonFunc("CheatSheet", [this](){ OpenCheatSheet_(); });
    AddButtonFunc("Done",       [this](){ Close("Done");     });
}

void HelpPanel::UpdateInterface() {
    SetFocus("Done");
}

void HelpPanel::OpenUserGuide_() {
    /// \todo Need to get version.
    Util::OpenURL("https://pss959.github.io/MakerVR-dist/0.2.1/UserGuide/");
}

void HelpPanel::OpenCheatSheet_() {
    /// \todo Need to get version.
    Util::OpenURL("https://pss959.github.io/MakerVR-dist/0.2.1/CheatSheet/");
}
