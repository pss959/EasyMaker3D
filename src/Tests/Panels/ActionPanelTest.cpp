#include "Enums/Action.h"
#include "Panels/ActionPanel.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ActionPanelTest : public PanelTestBase {
  protected:
    ActionPanelPtr panel;
    ActionPanelTest() { panel = InitPanel<ActionPanel>("ActionPanel"); }
};

TEST_F(ActionPanelTest, Defaults) {
    EXPECT_EQ(Action::kNone, panel->GetAction());
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(ActionPanelTest, SetAction) {
    panel->SetAction(Action::kUndo);
    EXPECT_EQ(Action::kUndo, panel->GetAction());
}

TEST_F(ActionPanelTest, Show) {
    EXPECT_FALSE(panel->IsShown());
    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());
    EXPECT_EQ(FindPane("Cancel"), panel->GetFocusedPane());
}
