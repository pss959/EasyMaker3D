#include "Panels/RadialMenuPanel.h"

#include <vector>

#include "Panes/BoxPane.h"
#include "Panes/RadioButtonPane.h"
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
    // Access relevant settings.
    const Settings &settings = GetSettings();
    left_info_  = RadialMenuInfo::CreateDefault();
    right_info_ = RadialMenuInfo::CreateDefault();
    left_info_->CopyFrom(settings.GetLeftRadialMenuInfo());
    right_info_->CopyFrom(settings.GetRightRadialMenuInfo());

    const auto &root_pane = GetPane();

    // Set up mode radio buttons.
    std::vector<RadioButtonPanePtr> mode_buttons;
    mode_buttons.push_back(root_pane->FindTypedPane<RadioButtonPane>("Mode0"));
    mode_buttons.push_back(root_pane->FindTypedPane<RadioButtonPane>("Mode1"));
    mode_buttons.push_back(root_pane->FindTypedPane<RadioButtonPane>("Mode2"));
    mode_buttons.push_back(root_pane->FindTypedPane<RadioButtonPane>("Mode3"));
    RadioButtonPane::CreateGroup(mode_buttons, GetModeIndex_(settings));

    // Set up both controller panes.
    left_menu_  = InitControllerPane_(Hand::kLeft,  *left_info_);
    right_menu_ = InitControllerPane_(Hand::kRight, *right_info_);

    SetFocus("Cancel");
}

RadialMenuPtr RadialMenuPanel::InitControllerPane_(Hand hand,
                                                   const RadialMenuInfo &info) {
    const std::string name = Util::EnumToWord(hand) + "Controller";
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

    for (auto &but: count_buttons)
        but->GetStateChanged().AddObserver(
            this, [&, hand](size_t index){ CountChanged_(hand, index); });

    // Set up the RadialMenu.
    auto menu = SG::FindTypedNodeUnderNode<RadialMenu>(*pane, "RadialMenu");
    menu->GetButtonClicked().AddObserver(
        this, [&, hand](size_t index, Action){ ButtonClicked_(hand, index); });
    menu->UpdateFromInfo(info);
    menu->SetEnabled(true);
    return menu;
}

void RadialMenuPanel::CountChanged_(Hand hand, size_t index) {
    const RadialMenuInfo::Count count =
        index == 0 ? RadialMenuInfo::Count::kCount2 :
        index == 1 ? RadialMenuInfo::Count::kCount4 :
        RadialMenuInfo::Count::kCount8;

    if (hand == Hand::kLeft) {
        left_info_->SetCount(count);
        left_menu_->UpdateFromInfo(*left_info_);
    }
    else {
        right_info_->SetCount(count);
        right_menu_->UpdateFromInfo(*right_info_);
    }
}

void RadialMenuPanel::ButtonClicked_(Hand hand, size_t index) {
    std::cerr << "XXXX Hand " << Util::EnumToWord(hand)
              << " index " << index << "\n";
}

void RadialMenuPanel::AcceptEdits_() {
    // XXXX
}
