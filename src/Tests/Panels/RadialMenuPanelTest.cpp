#include "Agents/SettingsAgent.h"
#include "Items/Settings.h"
#include "Panels/ActionPanel.h"
#include "Panels/RadialMenuPanel.h"
#include "Panes/TouchWrapperPane.h"
#include "Place/ClickInfo.h"
#include "Place/TouchInfo.h"
#include "SG/Search.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"
#include "Widgets/PushButtonWidget.h"

/// \ingroup Tests
class RadialMenuPanelTest : public PanelTestBase {
  protected:
    RadialMenuPanelPtr panel;
    RadialMenuPanelTest() {
        panel = InitPanel<RadialMenuPanel>("RadialMenuPanel");
    }
};

TEST_F(RadialMenuPanelTest, Defaults) {
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(RadialMenuPanelTest, Show) {
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
    EXPECT_EQ(FindPane("Cancel"), panel->GetFocusedPane());
}

TEST_F(RadialMenuPanelTest, Change) {
    panel->SetStatus(Panel::Status::kVisible);

    // Each RadialMenu instance in the RadialMenuPanel is wrapped in a
    // TouchWrapperPane.
    auto twp = FindTypedPane<TouchWrapperPane>("MenuBox");

    // Search for one of the buttons and simulate a click on it. This should
    // cause the ActionPanel to be opened to replace the RadialMenuPanel.
    auto but = SG::FindTypedNodeUnderNode<PushButtonWidget>(*twp, "Button_3");
    ClickInfo info;  // Contents do not matter.
    but->Click(info);

    // The RadialMenuPanel should be hidden and the ActionPanel should be
    // shown.
    EXPECT_ENUM_EQ(Panel::Status::kHidden, panel->GetStatus());
    auto action_panel = GetCurrentPanel();
    EXPECT_EQ("ActionPanel", action_panel->GetTypeName());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, action_panel->GetStatus());

    // Choose an action and Accept the ActionPanel. This should close it with
    // result "Accept" and reopen the RadialMenuPanel.
    ClickButtonPane("Undo");
    ClickButtonPane("Accept");
    EXPECT_EQ("Accept", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, action_panel->GetStatus());
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    // Clicking the Accept button in the RadialMenuPanel should change the
    // stored settings.
    const auto &settings = GetContext().settings_agent->GetSettings();
    EXPECT_ENUM_EQ(Action::kPaste,
                   settings.GetLeftRadialMenuInfo().GetButtonAction(3));
    ClickButtonPane("Accept");
    EXPECT_ENUM_EQ(Action::kUndo,
                   settings.GetLeftRadialMenuInfo().GetButtonAction(3));
}
