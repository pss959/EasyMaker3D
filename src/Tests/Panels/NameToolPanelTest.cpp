#include "Agents/NameAgent.h"
#include "Base/Event.h"
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
    EXPECT_TRUE(panel->IsMovable());
    EXPECT_TRUE(panel->IsResizable());
    EXPECT_FALSE(panel->CanGripHover());
    EXPECT_NULL(panel->GetGripWidget(Point2f(0, 0)));
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

    // All ToolPanel classes should Ignore valuator events.
    Event event;
    event.flags.Set(Event::Flag::kPosition1D);
    event.position1D = -.4f;
    EXPECT_FALSE(panel->HandleEvent(event));
}

TEST_F(NameToolPanelTest, Change) {
    panel->SetStatus(Panel::Status::kVisible);

    auto input = pi.SetTextInput("Input", "Some Name");
    EXPECT_EQ("Some Name", input->GetText());
    EXPECT_TRUE(input->IsTextValid());

    // Apply.
    pi.ClickButtonPane("Apply");
    EXPECT_EQ(1U,           GetChangeInfo().count);
    EXPECT_EQ("Name",       GetChangeInfo().name);
    EXPECT_EQ("kImmediate", GetChangeInfo().type);

    auto msg = FindTypedPane<TextPane>("Message");
    input = pi.SetTextInput("Input", " BadName");
    EXPECT_EQ(" BadName", input->GetText());
    EXPECT_FALSE(input->IsTextValid());
    EXPECT_TRUE(msg->GetText().contains("Invalid name"));

    GetContext().name_agent->Add("DupName");
    input = pi.SetTextInput("Input", "DupName");
    EXPECT_EQ("DupName", input->GetText());
    EXPECT_FALSE(input->IsTextValid());
    EXPECT_TRUE(msg->GetText().contains("Name is in use"));
}
