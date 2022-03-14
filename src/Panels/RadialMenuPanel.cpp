#include "Panels/RadialMenuPanel.h"

#include <vector>

#include "Items/RadialMenu.h"
#include "Panes/BoxPane.h"
#include "Panes/RadioButtonPane.h"
#include "RadialMenuInfo.h"
#include "SG/Search.h"
#include "Settings.h"
#include "Util/Enum.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

static size_t GetModeIndex_(const Settings &settings) {
    const std::string &mode_string = settings.GetRadialMenusMode();

    if (mode_string == "Disabled")
        return 0;
    else if (mode_string == "LeftForBoth")
        return 1;
    else if (mode_string == "RightForBoth")
        return 2;
    else
        return 3;
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// RadialMenuPanel functions.
// ----------------------------------------------------------------------------

void RadialMenuPanel::InitInterface() {
    AddButtonFunc("Cancel", [&](){ Close("Cancel"); });
    AddButtonFunc("Accept", [&](){ AcceptEdits_(); });
}

void RadialMenuPanel::UpdateInterface() {
    // Access all relevant settings.
    const Settings &settings = GetSettings();

    const auto &root_pane = GetPane();

    // Set up mode radio buttons.
    std::vector<RadioButtonPanePtr> mode_buttons;
    mode_buttons.push_back(root_pane->FindTypedPane<RadioButtonPane>("Mode0"));
    mode_buttons.push_back(root_pane->FindTypedPane<RadioButtonPane>("Mode1"));
    mode_buttons.push_back(root_pane->FindTypedPane<RadioButtonPane>("Mode2"));
    mode_buttons.push_back(root_pane->FindTypedPane<RadioButtonPane>("Mode3"));
    RadioButtonPane::CreateGroup(mode_buttons, GetModeIndex_(settings));

    // Set up both controller panes.
    InitControllerPane_("LeftController",  settings.GetLeftRadialMenuInfo());
    InitControllerPane_("RightController", settings.GetRightRadialMenuInfo());
}

void RadialMenuPanel::InitControllerPane_(const std::string &name,
                                          const RadialMenuInfo &info) {
    const auto &pane = GetPane()->FindTypedPane<BoxPane>(name);

    // Set up button count radio buttons.
    size_t count_index;
    switch (info.GetCount()) {
      case RadialMenuInfo::Count::kCount2: count_index = 0; break;
      case RadialMenuInfo::Count::kCount4: count_index = 1; break;
      case RadialMenuInfo::Count::kCount8: count_index = 2; break;
    }
    std::vector<RadioButtonPanePtr> count_buttons;
    count_buttons.push_back(pane->FindTypedPane<RadioButtonPane>("Count2"));
    count_buttons.push_back(pane->FindTypedPane<RadioButtonPane>("Count4"));
    count_buttons.push_back(pane->FindTypedPane<RadioButtonPane>("Count8"));
    RadioButtonPane::CreateGroup(count_buttons, count_index);

    // Set up the RadialMenu.
    auto menu = SG::FindTypedNodeUnderNode<RadialMenu>(*pane, "RadialMenu");
    menu->UpdateFromInfo(info);
    menu->SetEnabled(true);
}

void RadialMenuPanel::AcceptEdits_() {
    // XXXX
}
