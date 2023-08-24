#include "Panes/ButtonPane.h"
#include "Panels/DialogPanel.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"

class DialogPanelTest : public PanelTestBase {};

TEST_F(DialogPanelTest, SingleResponse) {
    auto panel = ReadRealPanel<DialogPanel>("DialogPanel");
    panel->SetMessage("Some message");
    panel->SetSingleResponse("Cool");
    auto but0 = FindTypedPane<ButtonPane>(*panel, "Button0");
    auto but1 = FindTypedPane<ButtonPane>(*panel, "Button1");
    EXPECT_TRUE(but0->IsEnabled());
    EXPECT_FALSE(but1->IsEnabled());
    EXPECT_EQ(but0, panel->GetFocusedPane());

    // Show the panel.
    EXPECT_FALSE(panel->IsShown());
    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());

    // Close the panel by simulating a click on the "Cool" button.
    ClickButtonPane(*panel, "Button0");
    EXPECT_EQ("Cool", GetCloseResult());
    EXPECT_FALSE(panel->IsShown());
}

TEST_F(DialogPanelTest, ChoiceResponse) {
    auto panel = ReadRealPanel<DialogPanel>("DialogPanel");
    panel->SetMessage("Some message");
    panel->SetChoiceResponse("Happy", "Sad", false);  // Focus Button1.
    auto but0 = FindTypedPane<ButtonPane>(*panel, "Button0");
    auto but1 = FindTypedPane<ButtonPane>(*panel, "Button1");
    EXPECT_TRUE(but0->IsEnabled());
    EXPECT_TRUE(but1->IsEnabled());
    EXPECT_EQ(but1, panel->GetFocusedPane());

    // Show the panel.
    EXPECT_FALSE(panel->IsShown());
    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());

    // Close the panel by simulating a click on the "Sad" button.
    ClickButtonPane(*panel, "Button1");
    EXPECT_EQ("Sad", GetCloseResult());
    EXPECT_FALSE(panel->IsShown());
}
