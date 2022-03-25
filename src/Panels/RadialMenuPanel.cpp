#include "Panels/RadialMenuPanel.h"

#include <vector>

#include "Items/RadialMenu.h"
#include "Managers/SettingsManager.h"
#include "Panels/ActionPanel.h"
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
    const auto &root_pane = GetPane();

    // Set up mode radio buttons.
    std::vector<RadioButtonPanePtr> mode_buttons;
    mode_buttons.push_back(root_pane->FindTypedPane<RadioButtonPane>("Mode0"));
    mode_buttons.push_back(root_pane->FindTypedPane<RadioButtonPane>("Mode1"));
    mode_buttons.push_back(root_pane->FindTypedPane<RadioButtonPane>("Mode2"));
    mode_buttons.push_back(root_pane->FindTypedPane<RadioButtonPane>("Mode3"));
    RadioButtonPane::CreateGroup(mode_buttons, 0);

    // Set up both controller panes.
    left_menu_  = InitControllerPane_(Hand::kLeft);
    right_menu_ = InitControllerPane_(Hand::kRight);

    AddButtonFunc("Cancel", [&](){ Close("Cancel"); });
    AddButtonFunc("Accept", [&](){ AcceptEdits_(); });
}

void RadialMenuPanel::UpdateInterface() {
    // Set up the RadialMenuInfo instances if not already done. If they already
    // exist, leave them alone so that changes made in the interface are
    // retained.
    if (! left_info_) {
        const Settings &settings = GetSettings();
        left_info_  = RadialMenuInfo::CreateDefault();
        right_info_ = RadialMenuInfo::CreateDefault();
        left_info_->CopyFrom(settings.GetLeftRadialMenuInfo());
        right_info_->CopyFrom(settings.GetRightRadialMenuInfo());
    }

    // Select the correct mode radio button.
    const size_t mode_index = GetModeIndex_(GetSettings());
    const std::string mode_name = "Mode" + Util::ToString(mode_index);
    GetPane()->FindTypedPane<RadioButtonPane>(mode_name)->SetState(true);

    // Update the controller panes.
    UpdateControllerPane_(Hand::kLeft,  *left_info_);
    UpdateControllerPane_(Hand::kRight, *right_info_);

    SetFocus("Cancel");
}

BoxPane & RadialMenuPanel::GetControllerPane_(Hand hand) {
    const std::string name = Util::EnumToWord(hand) + "ControllerPane";
    return *GetPane()->FindTypedPane<BoxPane>(name);
}

RadialMenuPtr RadialMenuPanel::InitControllerPane_(Hand hand) {
    const auto &pane = GetControllerPane_(hand);

    // Set up button count radio buttons.
    std::vector<RadioButtonPanePtr> count_buttons;
    count_buttons.push_back(pane.FindTypedPane<RadioButtonPane>("Count2"));
    count_buttons.push_back(pane.FindTypedPane<RadioButtonPane>("Count4"));
    count_buttons.push_back(pane.FindTypedPane<RadioButtonPane>("Count8"));
    RadioButtonPane::CreateGroup(count_buttons, 0);

    for (auto &but: count_buttons)
        but->GetStateChanged().AddObserver(
            this, [&, hand](size_t index){ CountChanged_(hand, index); });

    // Set up the RadialMenu.
    auto menu = SG::FindTypedNodeUnderNode<RadialMenu>(pane, "RadialMenu");
    menu->GetButtonClicked().AddObserver(
        this, [&, hand](size_t index, Action){ ButtonClicked_(hand, index); });
    menu->SetEnabled(true);
    return menu;
}

void RadialMenuPanel::UpdateControllerPane_(Hand hand,
                                            const RadialMenuInfo &info) {
    const auto &pane = GetControllerPane_(hand);

    // Set the correct number of buttons.
    const std::string &count_name = Util::EnumToWord(info.GetCount());
    pane.FindTypedPane<RadioButtonPane>(count_name)->SetState(true);

    // Update the RadialMenu.
    auto menu = SG::FindTypedNodeUnderNode<RadialMenu>(pane, "RadialMenu");
    menu->UpdateFromInfo(info);
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
    RadialMenuInfoPtr info = hand == Hand::kLeft ? left_info_ : right_info_;
    RadialMenuPtr     menu = hand == Hand::kLeft ? left_menu_ : right_menu_;

    auto init = [info, index](const PanelPtr &p){
        ASSERT(p->GetTypeName() == "ActionPanel");
        ActionPanel &ap = *Util::CastToDerived<ActionPanel>(p);
        ap.SetAction(info->GetButtonAction(index));
    };
    auto result = [info, menu, index](Panel &p, const std::string &res){
        if (res == "Accept") {
            ASSERT(p.GetTypeName() == "ActionPanel");
            ActionPanel &ap = static_cast<ActionPanel &>(p);
            const Action action = ap.GetAction();
            info->SetButtonAction(index, action);
            menu->ChangeButtonAction(index, action);
        }
    };
    GetContext().panel_helper->Replace("ActionPanel", init, result);
}

void RadialMenuPanel::AcceptEdits_() {
    // Copy the current settings.
    SettingsPtr new_settings = Settings::CreateDefault();
    new_settings->CopyFrom(GetSettings());

    // Update RadialMenuInfo for each hand.
    new_settings->SetLeftRadialMenuInfo(*left_info_);
    new_settings->SetRightRadialMenuInfo(*right_info_);

    GetContext().settings_manager->SetSettings(*new_settings);

    Close("Accept");
}
