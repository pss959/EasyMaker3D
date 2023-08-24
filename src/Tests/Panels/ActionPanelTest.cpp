#include "Enums/Action.h"
#include "Panels/ActionPanel.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"

class ActionPanelTest : public PanelTestBase {};

TEST_F(ActionPanelTest, Defaults) {
    auto panel = ReadRealPanel<ActionPanel>("ActionPanel");
    EXPECT_EQ(Action::kNone, panel->GetAction());
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(ActionPanelTest, SetAction) {
    auto panel = ReadRealPanel<ActionPanel>("ActionPanel");
    panel->SetAction(Action::kUndo);
    EXPECT_EQ(Action::kUndo, panel->GetAction());
}

TEST_F(ActionPanelTest, Show) {
    auto panel = ReadRealPanel<ActionPanel>("ActionPanel");
    EXPECT_FALSE(panel->IsShown());
    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());
    EXPECT_EQ(FindPane(*panel, "Cancel"), panel->GetFocusedPane());
}
