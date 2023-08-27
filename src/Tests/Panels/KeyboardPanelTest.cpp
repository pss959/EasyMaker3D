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
}

TEST_F(KeyboardPanelTest, Show) {
    EXPECT_FALSE(panel->IsShown());
    panel->SetIsShown(true);
    EXPECT_TRUE(panel->IsShown());
}

TEST_F(KeyboardPanelTest, Keys) {
    panel->SetIsShown(true);

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

    ClickButtonPane("Key_j");
    EXPECT_ENUM_EQ(TextAction::kInsert, last_action);
    EXPECT_EQ("j",                      last_insertion);

    ClickButtonPane("Key_left");
    EXPECT_ENUM_EQ(TextAction::kMovePrevious, last_action);
    EXPECT_EQ("",                             last_insertion);

    // The shift key changes internal state but does not result in an action.
    // Subsequent keys should be shifted.
    ClickButtonPane("Key_lshift");
    ClickButtonPane("Key_q");
    EXPECT_ENUM_EQ(TextAction::kInsert, last_action);
    EXPECT_EQ("Q",                      last_insertion);
}
