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
    NameToolPanelTest() {
        panel = InitPanel<NameToolPanel>("NameToolPanel");
        ObserveChanges(*panel);
    }
};

TEST_F(NameToolPanelTest, Defaults) {
    EXPECT_EQ(".", panel->GetName());
    EXPECT_NULL(panel->GetFocusedPane());
    EXPECT_FALSE(panel->IsCloseable());
}

TEST_F(NameToolPanelTest, SetName) {
    panel->SetName("Hello!");
    EXPECT_EQ("Hello!", panel->GetName());
    EXPECT_NULL(panel->GetFocusedPane());
}

TEST_F(NameToolPanelTest, Show) {
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
}

TEST_F(NameToolPanelTest, Change) {
    panel->SetStatus(Panel::Status::kVisible);

    auto input = SetTextInput("Input", "Some Name");
    EXPECT_EQ("Some Name", input->GetText());
    EXPECT_TRUE(input->IsTextValid());

    // Apply.
    ClickButtonPane("Apply");
    EXPECT_EQ(1U,           GetChangeInfo().count);
    EXPECT_EQ("Name",       GetChangeInfo().name);
    EXPECT_EQ("kImmediate", GetChangeInfo().type);

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
