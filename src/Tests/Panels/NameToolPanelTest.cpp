#include "Agents/NameAgent.h"
#include "Panels/NameToolPanel.h"
#include "Panes/TextInputPane.h"
#include "Panes/TextPane.h"
#include "Tests/Panels/ToolPanelTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class NameToolPanelTest : public ToolPanelTestBase {
  protected:
    NameToolPanelPtr panel;
    NameToolPanelTest() { panel = InitPanel<NameToolPanel>("NameToolPanel"); }
};

/// \ingroup Tests
class NameToolPanelTestWithText : public PanelTestBase {
  protected:
    NameToolPanelPtr panel;
    NameToolPanelTestWithText() : PanelTestBase(true) {
        panel = InitPanel<NameToolPanel>("NameToolPanel");
    }
};

TEST_F(NameToolPanelTest, Defaults) {
    EXPECT_EQ(".", panel->GetName());
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(NameToolPanelTest, SetName) {
    panel->SetName("Hello!");
    EXPECT_EQ("Hello!", panel->GetName());
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(NameToolPanelTest, Show) {
    EXPECT_FALSE(panel->IsShown());
    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());
}

TEST_F(NameToolPanelTestWithText, Change) {
    panel->SetSize(Vector2f(300, 300));  // Required for text sizing.
    panel->SetIsShown(true);

    auto input = SetTextInput("Input", "Some Name");
    EXPECT_EQ("Some Name", input->GetText());
    EXPECT_TRUE(input->IsTextValid());

    auto msg = FindTypedPane<TextPane>("Message");
    input = SetTextInput("Input", " BadName");
    EXPECT_EQ(" BadName", input->GetText());
    EXPECT_FALSE(input->IsTextValid());
    EXPECT_TRUE(msg->GetText().contains("Invalid name"));

    GetContext().name_agent->Add("DupName");
    input = SetTextInput("Input", "DupName");
    EXPECT_EQ("DupName", input->GetText());
    EXPECT_FALSE(input->IsTextValid());
    EXPECT_TRUE(msg->GetText().contains("Name is in use"));
}
