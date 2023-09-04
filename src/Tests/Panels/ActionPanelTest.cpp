#include "Enums/Action.h"
#include "Panels/ActionPanel.h"
#include "Panes/ScrollingPane.h"
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
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
    EXPECT_EQ(FindPane("Cancel"), panel->GetFocusedPane());
}

TEST_F(ActionPanelTest, Scroll) {
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());

    // Set the last action. The ActionPanel should scroll to the bottom.
    panel->SetAction(Action::kToggleShowEdges);
    panel->SetStatus(Panel::Status::kVisible);
    auto sp = FindTypedPane<ScrollingPane>("ScrollingPane");
    EXPECT_EQ(1, sp->GetScrollPosition());
}
