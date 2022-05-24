#include "Panels/HelpPanel.h"

#include "Util/URL.h"
#include "Util/VersionInfo.h"

void HelpPanel::InitInterface() {
    AddButtonFunc("UserGuide",  [this](){ OpenPage_("UserGuide");  });
    AddButtonFunc("CheatSheet", [this](){ OpenPage_("CheatSheet"); });
    AddButtonFunc("Done",       [this](){ Close("Done");     });
}

void HelpPanel::UpdateInterface() {
    SetFocus("Done");
}

void HelpPanel::OpenPage_(const std::string &page_name) {
    const std::string base_url = "https://pss959.github.io/MakerVR-dist/";
    Util::OpenURL(base_url + Util::kVersionString + "/" + page_name + "/");
}
