#include "Base/Event.h"
#include "Base/VirtualKeyboard.h"
#include "Panels/Panel.h"
#include "Panes/ButtonPane.h"
#include "Place/TouchInfo.h"
#include "Tests/Panels/PanelTestBase.h"
#include "Tests/Testing.h"
#include "Widgets/PushButtonWidget.h"

/// \ingroup Tests
class PanelTest : public PanelTestBase {
  protected:
    /// Derived Panel class that allows for testing of focus changes.
    class TestPanel : public Panel {
      protected:
        TestPanel() {}
        virtual void InitInterface() override {}
        friend class Parser::Registry;
    };
    DECL_SHARED_PTR(TestPanel);

    TestPanelPtr panel;

    PanelTest();
};

PanelTest::PanelTest() {
    // The TestPanel has to be registered and defined.
    Parser::Registry::AddType<TestPanel>("TestPanel");

    const Str extra_contents = R"(
TestPanel "TestPanel" {
  pane: BoxPane {
    CONSTANTS: [BORDER: "CLONE \"T_Border\" \"Border\""],
    TEMPLATES: [
      <"nodes/templates/Border.emd">,
      <"nodes/templates/Panes/ButtonPane.emd">,
      <"nodes/templates/Panes/SliderPane.emd">,
    ],
    panes: [
      CLONE "T_ButtonPane" "Button0" { min_size: 100 20 }
      CLONE "T_ButtonPane" "Button1" { min_size: 100 20 }
      CLONE "T_ButtonPane" "Button2" { min_size: 100 20 }
      CLONE "T_ButtonPane" "Button3" { disabled_flags: "kTraversal" }
      CLONE "T_SliderPane" "Slider"  { min_size: 100 20 }
    ]
  }
}
)";

    panel = InitPanel<TestPanel>("TestPanel", extra_contents);
}

TEST_F(PanelTest, IsValid) {
    SetParseTypeName("TestPanel");
    TestInvalid("", "Missing Pane");
}

TEST_F(PanelTest, Size) {
    EXPECT_EQ(Vector2f(100, 80), panel->GetMinSize());
    EXPECT_EQ(Vector2f(100, 80), panel->GetSize());
}

TEST_F(PanelTest, ActivateButton) {
    bool clicked = false;
    auto but0 = FindTypedPane<ButtonPane>("Button0");
    but0->GetButton().GetClicked().AddObserver(
        "key", [&](const ClickInfo &){ clicked = true; });

    panel->SetStatus(Panel::Status::kVisible);

    // Pressing "Enter" should activate the the focused button (Button0).
    Event event;
    event.device = Event::Device::kKeyboard;
    event.flags.Set(Event::Flag::kKeyPress);
    event.key_name = "Enter";
    EXPECT_FALSE(clicked);
    EXPECT_TRUE(panel->HandleEvent(event));
    EXPECT_TRUE(clicked);
}

TEST_F(PanelTest, HeadsetButton) {
    auto &vk = *GetContext().virtual_keyboard;
    EXPECT_FALSE(vk.IsVisible());

    Event event;
    event.device = Event::Device::kHeadset;
    event.button = Event::Button::kHeadset;

    // The VirtualKeyboard should become visible when the headset button is
    // pressed.
    event.flags.Set(Event::Flag::kButtonPress);
    EXPECT_TRUE(panel->HandleEvent(event));
    EXPECT_TRUE(vk.IsVisible());

    // And not visible when the headset button is released.
    event.flags.SetAll(false);
    event.flags.Set(Event::Flag::kButtonRelease);
    EXPECT_TRUE(panel->HandleEvent(event));
    EXPECT_FALSE(vk.IsVisible());
}

TEST_F(PanelTest, Valuator) {
    // All non-ToolPanel Panel classes should trap valuator events.
    Event event;
    event.flags.Set(Event::Flag::kPosition1D);
    event.position1D = -.4f;
    EXPECT_TRUE(panel->HandleEvent(event));
}

TEST_F(PanelTest, FocusEvents) {
    auto but0 = FindTypedPane<ButtonPane>("Button0");
    auto but1 = FindTypedPane<ButtonPane>("Button1");
    auto but2 = FindTypedPane<ButtonPane>("Button2");

    panel->SetStatus(Panel::Status::kVisible);

    Event event;
    event.device = Event::Device::kKeyboard;
    event.flags.Set(Event::Flag::kKeyPress);
    event.key_name = "Tab";

    // Initial focus should be on Button0.
    EXPECT_EQ(but0, panel->GetFocusedPane());

    // Tab should focus on Button1.
    EXPECT_TRUE(panel->HandleEvent(event));
    EXPECT_EQ(but1, panel->GetFocusedPane());

    // Another Tab should focus on Button2.
    EXPECT_TRUE(panel->HandleEvent(event));
    EXPECT_EQ(but2, panel->GetFocusedPane());

    // Another Tab should focus back on Button0 - Skip Button3, which is
    // disabled, and Slider, which is not focusable.
    EXPECT_TRUE(panel->HandleEvent(event));
    EXPECT_EQ(but0, panel->GetFocusedPane());

    // Shift-Tab should move up to Button2.
    event.modifiers.Set(Event::ModifierKey::kShift);
    EXPECT_TRUE(panel->HandleEvent(event));
    EXPECT_EQ(but2, panel->GetFocusedPane());

    // Disable Button0 and Button1; Tab and Shift-Tab should stay on Button2.
    but0->SetInteractionEnabled(false);
    but1->SetInteractionEnabled(false);
    EXPECT_TRUE(panel->HandleEvent(event));
    EXPECT_EQ(but2, panel->GetFocusedPane());
    event.modifiers.SetAll(false);
    EXPECT_TRUE(panel->HandleEvent(event));
    EXPECT_EQ(but2, panel->GetFocusedPane());

    // Test direct change to focus. [Note: SetFocusedPane() is public so that
    // SnapScript can make changes, but the main app does not use it.]
    but0->SetInteractionEnabled(true);
    panel->SetFocusedPane(but0);
    EXPECT_EQ(but0, panel->GetFocusedPane());
}

TEST_F(PanelTest, GetTouchedPaneWidget) {
    TouchInfo info;
    info.position.Set(0, 0, 0);
    info.radius = .01f;
    info.root_node = panel;

    EXPECT_NULL(panel->GetTouchedPaneWidget(info));
}
