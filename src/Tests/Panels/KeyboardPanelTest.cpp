#include "Base/VirtualKeyboard.h"
#include "Panels/KeyboardPanel.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class KeyboardPanelTest : public PanelTestBase {
  protected:
    KeyboardPanelPtr panel;
    KeyboardPanelTest() { panel = InitPanel<KeyboardPanel>("KeyboardPanel"); }
};

TEST_F(KeyboardPanelTest, Defaults) {
    EXPECT_NULL(panel->GetFocusedPane());
    EXPECT_TRUE(panel->IsMovable());
    EXPECT_FALSE(panel->IsResizable());
    EXPECT_FALSE(panel->CanGripHover());
    EXPECT_NULL(panel->GetGripWidget(Point2f(0, 0)));
}

TEST_F(KeyboardPanelTest, Show) {
    EXPECT_ENUM_EQ(Panel::Status::kUnattached, panel->GetStatus());
    panel->SetStatus(Panel::Status::kVisible);
    EXPECT_ENUM_EQ(Panel::Status::kVisible, panel->GetStatus());
}

TEST_F(KeyboardPanelTest, Keys) {
    panel->SetStatus(Panel::Status::kVisible);

    // Create a VirtualKeyboard that accepts the key presses.
    auto &vk = *GetContext().virtual_keyboard;

    // VirtualKeyboard editing callbacks.
    TextAction last_action = TextAction::kCancel;
    Str        last_insertion;
    const auto insert = [&](const Str &s){
        last_action    = TextAction::kInsert;
        last_insertion = s;
    };
    const auto action = [&](TextAction act){
        last_action    = act;
        last_insertion = "";
    };
    vk.GetInsertion().AddObserver("key", insert);
    vk.GetAction().AddObserver("key",    action);
    vk.SetIsActive(true);
    vk.SetIsVisible(true);
    EXPECT_ENUM_EQ(TextAction::kCancel, last_action);
    EXPECT_EQ("",                       last_insertion);

    pi.ClickButtonPane("Key_j");
    EXPECT_ENUM_EQ(TextAction::kInsert, last_action);
    EXPECT_EQ("j",                      last_insertion);

    pi.ClickButtonPane("Key_left");
    EXPECT_ENUM_EQ(TextAction::kMovePrevious, last_action);
    EXPECT_EQ("",                             last_insertion);

    // The shift key changes internal state but does not result in an action.
    // Subsequent keys should be shifted.
    pi.ClickButtonPane("Key_lshift");
    pi.ClickButtonPane("Key_q");
    EXPECT_ENUM_EQ(TextAction::kInsert, last_action);
    EXPECT_EQ("Q",                      last_insertion);
}
