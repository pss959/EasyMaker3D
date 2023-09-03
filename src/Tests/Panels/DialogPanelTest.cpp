#include "Panes/ButtonPane.h"
#include "Panels/DialogPanel.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class DialogPanelTest : public PanelTestBase {
  protected:
    DialogPanelPtr panel;
    DialogPanelTest() { panel = InitPanel<DialogPanel>("DialogPanel"); }
};

TEST_F(DialogPanelTest, SingleResponse) {
    panel->SetMessage("Some message");
    panel->SetSingleResponse("Cool");
    auto but0 = FindTypedPane<ButtonPane>("Button0");
    auto but1 = FindTypedPane<ButtonPane>("Button1");
    EXPECT_TRUE(but0->IsEnabled());
    EXPECT_FALSE(but1->IsEnabled());
    EXPECT_EQ(but0, panel->GetFocusedPane());

    // Show the panel.
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    // Close the panel by simulating a click on the "Cool" button.
    ClickButtonPane("Button0");
    EXPECT_EQ("Cool", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
}

TEST_F(DialogPanelTest, ChoiceResponse) {
    panel->SetMessage("Some message");
    panel->SetChoiceResponse("Happy", "Sad", false);  // Focus Button1.
    auto but0 = FindTypedPane<ButtonPane>("Button0");
    auto but1 = FindTypedPane<ButtonPane>("Button1");
    EXPECT_TRUE(but0->IsEnabled());
    EXPECT_TRUE(but1->IsEnabled());
    EXPECT_EQ(but1, panel->GetFocusedPane());

    // Show the panel.
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());

    // Close the panel by simulating a click on the "Sad" button.
    ClickButtonPane("Button1");
    EXPECT_EQ("Sad", GetCloseResult());
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
}
