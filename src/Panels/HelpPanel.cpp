//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panels/HelpPanel.h"

#include "SG/Search.h"
#include "Panes/TextPane.h"
#include "Util/Tuning.h"
#include "Util/URL.h"

void HelpPanel::InitInterface() {
    AddButtonFunc("UserGuide",  [this](){ OpenPage_("UserGuide");  });
    AddButtonFunc("CheatSheet", [this](){ OpenPage_("CheatSheet"); });
    AddButtonFunc("Issue",      [this](){ OpenIssue_();            });
    AddButtonFunc("Done",       [this](){ Close("Done");           });

    // Set the description text.
    auto &root_pane = GetPane();
    auto desc_pane  = root_pane->FindTypedSubPane<TextPane>("Description");
    desc_pane->SetText(TK::kApplicationName + " Version " + TK::kVersionString +
                       "\nCopyright " + TK::kCopyright);
}

void HelpPanel::UpdateInterface() {
    SetFocus("Done");
}

void HelpPanel::OpenPage_(const Str &page_name) {
    Util::OpenURL(TK::kPublicDocBaseURL + "/" + TK::kVersionString + "/" +
                  page_name);
}

void HelpPanel::OpenIssue_() {
    Util::OpenURL(TK::kGithubURL + "/issues");
}
