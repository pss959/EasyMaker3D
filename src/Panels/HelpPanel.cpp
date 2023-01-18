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
    auto &root_pane = GetPane();
    auto title_pane = root_pane->FindTypedPane<TextPane>("Title");
    auto desc_pane  = root_pane->FindTypedPane<TextPane>("Description");
    title_pane->SetText(TK::kApplicationName + " Help");
    desc_pane->SetText(TK::kApplicationName + " Version " + TK::kVersionString +
                       "\nCopyright " + TK::kCopyright);
}

void HelpPanel::UpdateInterface() {
    SetFocus("Done");
}

void HelpPanel::OpenPage_(const std::string &page_name) {
    Util::OpenURL(TK::kPublicDocBaseURL + TK::kVersionString + "/" +
                  page_name + "/");
}
