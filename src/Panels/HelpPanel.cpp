#include "Panels/HelpPanel.h"

#include <string>

#include "Base/Tuning.h"
#include "SG/Search.h"
#include "Panes/TextPane.h"
#include "Util/URL.h"

void HelpPanel::InitInterface() {
    AddButtonFunc("UserGuide",  [this](){ OpenPage_("UserGuide");  });
    AddButtonFunc("CheatSheet", [this](){ OpenPage_("CheatSheet"); });
    AddButtonFunc("Done",       [this](){ Close("Done");     });

    // Set the title text and description.
    const std::string app_name = TK::kApplicationName;
    const std::string version  = TK::kVersionString;
    auto &root_pane = GetPane();
    auto title_pane = root_pane->FindTypedPane<TextPane>("Title");
    auto desc_pane  = root_pane->FindTypedPane<TextPane>("Description");
    title_pane->SetText(app_name + " Help");
    desc_pane->SetText(app_name + " Version " + version +
                       "\nCopyright 2022 Paul S. Strauss");
}

void HelpPanel::UpdateInterface() {
    SetFocus("Done");
}

void HelpPanel::OpenPage_(const std::string &page_name) {
    const std::string base_url = TK::kPublicDocBaseURL;
    Util::OpenURL(base_url + TK::kVersionString + "/" + page_name + "/");
}
