#include "Base/VirtualKeyboard.h"
#include "Enums/TextAction.h"
#include "Tests/Testing.h"

TEST(VirtualKeyboardTest, ActiveAndVisible) {
    VirtualKeyboard vk;

    bool show_hide_state = false;

    const auto show_hide = [&](bool is_shown){ show_hide_state = is_shown; };
    vk.SetShowHideFunc(show_hide);

    EXPECT_FALSE(vk.IsActive());
    EXPECT_FALSE(vk.IsVisible());

    vk.SetIsActive(true);
    EXPECT_TRUE(vk.IsActive());
    EXPECT_FALSE(vk.IsVisible());
    EXPECT_FALSE(show_hide_state);

    vk.SetIsVisible(true);
    EXPECT_TRUE(vk.IsActive());
    EXPECT_TRUE(vk.IsVisible());
    EXPECT_TRUE(show_hide_state);

    vk.SetIsVisible(false);
    EXPECT_TRUE(vk.IsActive());
    EXPECT_FALSE(vk.IsVisible());
    EXPECT_FALSE(show_hide_state);

    vk.SetIsVisible(true);
    vk.SetIsActive(false);
    EXPECT_FALSE(vk.IsActive());
    EXPECT_TRUE(vk.IsVisible());
    EXPECT_FALSE(show_hide_state);
}

TEST(VirtualKeyboardTest, EditText) {
    bool        got_action = false;
    TextAction  last_action;
    std::string last_insertion;

    // Editing callbacks.
    const auto insert = [&](const std::string &s){ last_insertion = s; };
    const auto action = [&](TextAction act){
        got_action  = true;
        last_action = act;
    };

    VirtualKeyboard vk;
    vk.GetInsertion().AddObserver("x", insert);
    vk.GetAction().AddObserver("x",    action);

    EXPECT_FALSE(got_action);
    EXPECT_EQ("", last_insertion);

    // No changes while not active or not visible.
    vk.InsertText("blah");
    EXPECT_FALSE(got_action);
    EXPECT_EQ("", last_insertion);
    vk.ProcessTextAction(TextAction::kToggleShift);
    EXPECT_FALSE(got_action);
    EXPECT_EQ("", last_insertion);

    vk.SetIsActive(true);
    vk.SetIsVisible(false);
    vk.InsertText("foo");
    EXPECT_FALSE(got_action);
    EXPECT_EQ("", last_insertion);
    vk.ProcessTextAction(TextAction::kToggleShift);
    EXPECT_FALSE(got_action);
    EXPECT_EQ("", last_insertion);

    vk.SetIsActive(false);
    vk.SetIsVisible(true);
    vk.InsertText("hey");
    EXPECT_FALSE(got_action);
    EXPECT_EQ("", last_insertion);
    vk.ProcessTextAction(TextAction::kToggleShift);
    EXPECT_FALSE(got_action);
    EXPECT_EQ("", last_insertion);

    // Expect changes while active and visible.
    vk.SetIsActive(true);
    vk.SetIsVisible(true);
    vk.InsertText("blort");
    EXPECT_FALSE(got_action);
    EXPECT_EQ("blort", last_insertion);
    vk.ProcessTextAction(TextAction::kToggleShift);
    EXPECT_EQ("blort", last_insertion);
    EXPECT_TRUE(got_action);
    EXPECT_ENUM_EQ(TextAction::kToggleShift, last_action);
    vk.InsertText("more");
    EXPECT_EQ("more", last_insertion);
    vk.ProcessTextAction(TextAction::kSelectAll);
    EXPECT_TRUE(got_action);
    EXPECT_ENUM_EQ(TextAction::kSelectAll, last_action);
}
