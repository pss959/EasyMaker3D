#include "Base/Event.h"
#include "Base/VirtualKeyboard.h"
#include "Panes/IPaneInteractor.h"
#include "Panes/TextInputPane.h"
#include "Place/ClickInfo.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Widgets/DragTester.h"
#include "Util/Assert.h"
#include "Widgets/DraggableWidget.h"

/// \ingroup Tests
class TextInputPaneTest : public PaneTestBase {
  protected:
    /// Sets up and returns a TextInputPane
    TextInputPane & GetTextInputPane(const Str &contents = "");

    /// Processes a key press for the stored TextInputPane.
    void ProcessKey(const Str &str, bool exp_return_val = true);

    // Processes multiple key presses.
    void ProcessKeys(const StrVec &str_vec) {
        for (const auto &str: str_vec)
            ProcessKey(str);
    }

  private:
    TextInputPanePtr input_;
};

TextInputPane & TextInputPaneTest::GetTextInputPane(const Str &contents) {
    input_ = ReadRealPane<TextInputPane>("TextInputPane", contents);

    input_->SetLayoutSize(Vector2f(200, 20));  // Required for text sizing.

    return *input_;
}

void TextInputPaneTest::ProcessKey(const Str &str, bool exp_return_val) {
    Event event;
    event.device = Event::Device::kKeyboard;
    event.flags.Set(Event::Flag::kKeyPress);

    Str error;
    EXPECT_TRUE(Event::ParseKeyString(str, event.modifiers,
                                      event.key_name, error))
        << " Parse error: " << error << " for '" << str << "'";

    event.key_text = Event::BuildKeyText(event.modifiers, event.key_name);

    const bool ret = input_->GetInteractor()->HandleEvent(event);
    EXPECT_EQ(exp_return_val, ret) << " for '" << str << "'";
}

// ----------------------------------------------------------------------------
// Tests.
// ----------------------------------------------------------------------------

TEST_F(TextInputPaneTest, Defaults) {
    auto &input = GetTextInputPane();
    EXPECT_EQ(".", input.GetText());
    EXPECT_TRUE(input.IsTextValid());
    auto inter = input.GetInteractor();
    EXPECT_NOT_NULL(inter);
    EXPECT_NOT_NULL(inter->GetActivationWidget());
    EXPECT_EQ(input.GetBorder(), inter->GetFocusBorder());
    EXPECT_TRUE(inter->CanFocus(FocusReason::kInitialFocus));
    EXPECT_TRUE(inter->CanFocus(FocusReason::kActivation));
    EXPECT_TRUE(inter->CanFocus(FocusReason::kMove));
}

TEST_F(TextInputPaneTest, InitialText) {
    auto &input = GetTextInputPane(R"(initial_text: "Hello there")");
    EXPECT_EQ("Hello there", input.GetText());
    EXPECT_TRUE(input.IsTextValid());

    input.SetInitialText("Something else");
    EXPECT_EQ("Something else", input.GetText());
    EXPECT_TRUE(input.IsTextValid());
}

TEST_F(TextInputPaneTest, EditAndValidate) {
    auto &input = GetTextInputPane();
    auto inter = input.GetInteractor();

    input.SetInitialText("Abcd efgh");
    EXPECT_EQ("Abcd efgh", input.GetText());

    // Nothing happens until activation.
    EXPECT_FALSE(inter->IsActive());
    ProcessKey("Backspace", false);
    EXPECT_EQ("Abcd efgh", input.GetText());

    inter->Activate();
    EXPECT_TRUE(inter->IsActive());

    // kDeleteSelected (last character).
    ProcessKey("Backspace");
    EXPECT_EQ("Abcd efg", input.GetText());

    // kDeleteAll.
    ProcessKey("Ctrl-Backspace");
    EXPECT_EQ("", input.GetText());

    // kUndo.
    ProcessKey("Ctrl-z");
    EXPECT_EQ("Abcd efg", input.GetText());

    // kSelectAll and kDeleteSelected.
    ProcessKeys(StrVec{"Ctrl-a", "Backspace"});
    EXPECT_EQ("", input.GetText());

    // Insert characters.
    ProcessKeys(StrVec{"Shift-m","n", "o", "Space", "P", "q", "r"});
    EXPECT_EQ("Mno Pqr", input.GetText());

    // kMovePrevious, kSelectPrevious, and replace by inserting.
    ProcessKeys(StrVec{"Left", "Left", "Shift-Left", "Z"});
    EXPECT_EQ("Mno Zqr", input.GetText());

    // kMoveToStart, kMoveNext, kSelectNext, and replace by inserting.
    ProcessKeys(StrVec{"Up", "Right", "Shift-Right", "W"});
    EXPECT_EQ("MWo Zqr", input.GetText());

    // kMoveToEnd, insert, kUndo, kRedo
    ProcessKeys(StrVec{"Down", "S"});
    EXPECT_EQ("MWo ZqrS", input.GetText());
    ProcessKey("Ctrl-z");
    EXPECT_EQ("MWo Zqr", input.GetText());
    ProcessKey("Shift-Ctrl-z");
    EXPECT_EQ("MWo ZqrS", input.GetText());

    // Another kRedo has no effect.
    ProcessKey("Shift-Ctrl-z");
    EXPECT_EQ("MWo ZqrS", input.GetText());

    // kSelectToStart and replace by inserting.
    ProcessKeys(StrVec{"Shift-Up", "F", "g", "h", "i"});
    EXPECT_EQ("Fghi", input.GetText());

    // kMovePrevious twice and kDeleteToEnd.
    ProcessKeys(StrVec{"Left", "Left", "Ctrl-k"});
    EXPECT_EQ("Fg", input.GetText());

    // kDeleteToStart.
    ProcessKey("Shift-Ctrl-k");
    EXPECT_EQ("", input.GetText());

    // Insert some new text for more testing.
    ProcessKeys(StrVec{"A", "b", "c", "d", "Space", "e", "f", "g", "h"});
    EXPECT_EQ("Abcd efgh", input.GetText());

    // kSelectAll + kMovePrevious => moves to start; then kDeleteNext.
    ProcessKeys(StrVec{"Ctrl-a", "Left", "Ctrl-d"});
    EXPECT_EQ("bcd efgh", input.GetText());

    // kSelectAll + kMoveNext => moves to end; then kDeletePrevious.
    ProcessKeys(StrVec{"Ctrl-a", "Right", "Ctrl-h"});
    EXPECT_EQ("bcd efg", input.GetText());

    // kSelectAll, kSelectNone, kDeleteSelected should have no effect.
    ProcessKeys(StrVec{"Ctrl-a", "Shift-Ctrl-a", "Backspace"});
    EXPECT_EQ("bcd efg", input.GetText());

    // kMoveToEnd, move left 3 chars, kSelectToEnd, and replace by inserting.
    ProcessKeys(StrVec{"Down", "Left", "Left", "Left", "Shift-Down", "Q"});
    EXPECT_EQ("bcd Q", input.GetText());

    // Reset string to test extending selection. ---------
    input.SetInitialText("Abcd efgh");

    // kMoveToStart, kSelectNext twice, kSelectPrevious should select first
    // char.
    ProcessKeys(StrVec{"Up", "Shift-Right", "Shift-Right", "Shift-Left", "S"});
    EXPECT_EQ("Sbcd efgh", input.GetText());

    // kMoveToEnd, kSelectPrevious twice, kSelectNext should select last char.
    ProcessKeys(StrVec{"Down", "Shift-Left", "Shift-Left", "Shift-Right", "U"});
    EXPECT_EQ("Sbcd efgU", input.GetText());

    // kAccept should deactivate but return false.
    ProcessKey("Enter", false);
    EXPECT_FALSE(inter->IsActive());
    EXPECT_EQ("Sbcd efgU", input.GetText());

    // Test kCancel; make sure it resets the text to the initial text and
    // return false.
    inter->Activate();
    EXPECT_TRUE(inter->IsActive());
    ProcessKeys(StrVec{"Ctrl-a", "Z"});
    EXPECT_EQ("Z", input.GetText());
    ProcessKey("Escape", false);
    EXPECT_FALSE(inter->IsActive());
    EXPECT_EQ("Abcd efgh", input.GetText());
}

TEST_F(TextInputPaneTest, Validate) {
    // Simple validation function for testing.
    auto valid_func = [](const Str &s){ return ! s.contains('!'); };

    auto &input = GetTextInputPane();

    input.SetValidationFunc(valid_func);

    // TextInputPane has to be active for validation.
    input.GetInteractor()->Activate();

    input.SetInitialText("Hello");
    EXPECT_EQ("Hello", input.GetText());
    EXPECT_TRUE(input.IsTextValid());

    // Move to end and add a "!" to make it invalid.
    ProcessKeys(StrVec{"Down", "!"});
    EXPECT_EQ("Hello!", input.GetText());
    EXPECT_FALSE(input.IsTextValid());

    // Deactivation while invalid should reset the text to the initial text.
    input.GetInteractor()->Deactivate();
    EXPECT_EQ("Hello", input.GetText());
    EXPECT_TRUE(input.IsTextValid());
}

TEST_F(TextInputPaneTest, ClickAndDrag) {
    auto &input = GetTextInputPane();
    input.SetInitialText("Abcdefghijklmnop");
    input.GetInteractor()->Activate();
    EXPECT_EQ("Abcdefghijklmnop", input.GetText());

    auto dw = std::dynamic_pointer_cast<DraggableWidget>(
        input.GetInteractor()->GetActivationWidget());
    EXPECT_NOT_NULL(dw);

    DragTester dt(dw);
    dt.SetRayDirection(-Vector3f::AxisZ());

    // Drag to the right should select some characters. Replace them with "X".
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(.5f, 0, 0));
    ProcessKey("X");
    EXPECT_EQ("AbcdefghX", input.GetText());

    // Try a touch drag.
    dt.ApplyTouchDrag(Point3f(0, 0, 0), Point3f(-.1f, 0, 0));
    ProcessKey("Y");
    EXPECT_EQ("AbcdefYX", input.GetText());

    // Clicking in a different spot and deleting the next character.
    ClickInfo info;
    info.hit.point.Set(-.35f, 0, 0);
    dw->Click(info);
    ProcessKey("Ctrl-d");
    EXPECT_EQ("AbdefYX", input.GetText());

    // Drag to the roughly same place is like a click.
    dt.ApplyMouseDrag(Point3f(-.2f, 0, 0), Point3f(-.201f, 0, 0));
    ProcessKey("Z");
    EXPECT_EQ("AbdefZYX", input.GetText());
}

TEST_F(TextInputPaneTest, VirtualKeyboard) {
    // VirtualKeyboard needs to be active and visible to have effect.
    VirtualKeyboardPtr vk(new VirtualKeyboard);
    vk->SetIsActive(true);
    vk->SetIsVisible(true);

    auto &input = GetTextInputPane();
    input.GetInteractor()->SetVirtualKeyboard(vk);
    input.SetInitialText("Abcdefg");

    input.GetInteractor()->Activate();

    vk->ProcessTextAction(TextAction::kMoveToEnd);
    vk->ProcessTextAction(TextAction::kDeletePrevious);
    EXPECT_EQ("Abcdef", input.GetText());

    // kToggleShift has no effect.
    vk->ProcessTextAction(TextAction::kToggleShift);
    EXPECT_EQ("Abcdef", input.GetText());

    TEST_THROW(vk->ProcessTextAction(TextAction::kInsert),
               AssertException, "kInsert should use InsertText");

    input.GetInteractor()->Deactivate();
}
