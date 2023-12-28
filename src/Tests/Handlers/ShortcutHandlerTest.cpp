#include "Agents/ActionAgent.h"
#include "Base/Event.h"
#include "Handlers/ShortcutHandler.h"
#include "Tests/TempFile.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"
#include "Tests/Util/FakeFileSystem.h"
#include "Util/String.h"

/// \ingroup Tests
class ShortcutHandlerTest : public TestBase {
  protected:
    /// Derived ActionAgent that allows for testing of Action processing when
    /// handling events - it saves the most recent action.
    class TestActionAgent : public ActionAgent {
      public:
        /// Last action that was applied.
        Action last_action = Action::kNone;
        virtual void Reset() override {};
        virtual void UpdateFromSessionState(
            const SessionState &state) override {};
        virtual bool CanApplyAction(Action action) const override {
            return true;
        }
        virtual void ApplyAction(Action action) override {
            last_action = action;
        }
    };
    DECL_SHARED_PTR(TestActionAgent);

    ShortcutHandler sh;     ///< ShortcutHandler common to all tests.
    Str             error;  ///< Error string common to all custom tests.

    /// \name Custom shortcut helpers.
    ///@{

    bool AddInput(const Str &input) {
        return sh.AddCustomShortcutsFromString(input, error);
    }

    void TestErrorString(const Str &pattern) {
        TestString(error, pattern);
    }

    void TestErrorLine(int line) {
        TestString(error, "line " + Util::ToString(line));
    }

    void TestShortcutString(Action action, const Str &pattern) {
        Str ks, cs;
        sh.GetShortcutStrings(action, ks, cs);
        TestString(ks, pattern);
    }

    void TestString(const Str &s, const Str &pattern) {
        EXPECT_TRUE(s.contains(pattern))
            << " s = '" << s << "' pattern = '" << pattern << "'";
    }

    ///@}
};

TEST_F(ShortcutHandlerTest, HandleEvent) {
    sh.SetAppShortcutsEnabled(true);
    // Don't let Debug shortcuts interfere with testing; they require much more
    // setup to actually do anything.
    sh.SetDebugShortcutsEnabled(false);

    // Install an ActionAgent that tracks the last action taken.
    TestActionAgentPtr taa(new TestActionAgent);
    sh.SetActionAgent(taa);

    Str error;

    Event event;
    event.device = Event::Device::kKeyboard;
    event.flags.Set(Event::Flag::kKeyPress);

    EXPECT_ENUM_EQ(Action::kNone, taa->last_action);

    // Quit shortcut.
    EXPECT_TRUE(Event::ParseKeyString("Ctrl-q", event.modifiers,
                                      event.key_name, error));
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, sh.HandleEvent(event));
    EXPECT_ENUM_EQ(Action::kQuit, taa->last_action);

    // Delete shortcut
    EXPECT_TRUE(Event::ParseKeyString("Backspace", event.modifiers,
                                      event.key_name, error));
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, sh.HandleEvent(event));
    EXPECT_ENUM_EQ(Action::kDelete, taa->last_action);

    // Nonexistent shortcut
    EXPECT_TRUE(Event::ParseKeyString("Ctrl-Shift-0", event.modifiers,
                                      event.key_name, error));
    EXPECT_ENUM_EQ(Handler::HandleCode::kNotHandled, sh.HandleEvent(event));
    EXPECT_ENUM_EQ(Action::kDelete, taa->last_action);  // No change.

    // Controller shortcuts.
    event.device = Event::Device::kLeftController;
    event.flags.SetAll(false);
    event.flags.Set(Event::Flag::kButtonPress);
    event.button = Event::Button::kDown;
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, sh.HandleEvent(event));
    EXPECT_ENUM_EQ(Action::kDecreasePrecision, taa->last_action);
    event.device = Event::Device::kRightController;
    event.button = Event::Button::kUp;
    EXPECT_ENUM_EQ(Handler::HandleCode::kHandledStop, sh.HandleEvent(event));
    EXPECT_ENUM_EQ(Action::kIncreasePrecision, taa->last_action);

    // Has no effect.
    sh.Reset();
}

TEST_F(ShortcutHandlerTest, CustomEmpty) {
    EXPECT_TRUE(AddInput(""));
    EXPECT_TRUE(error.empty());
}

TEST_F(ShortcutHandlerTest, CustomComments) {
    const Str input =
        "# Hello there\n"
        "     # Otherwise blank   \n"
        "##############\n";
    EXPECT_TRUE(AddInput(input));
    EXPECT_TRUE(error.empty());
}

TEST_F(ShortcutHandlerTest, CustomValid) {
    const Str input =
        "# Header comment\n"
        "  a Cut\n"
        "  b        Copy\n"
        "  Shift-c   Paste\n"
        "  Ctrl-Shift-d Delete\n"
        "  Alt-Ctrl-Shift-e Undo\n"
        "  Alt-Ctrl-Shift-Alt-f Redo\n"
        "  Shift-Alt-Ctrl-Shift-Alt-g Quit\n";
    EXPECT_TRUE(AddInput(input));
    EXPECT_TRUE(error.empty());

    TestShortcutString(Action::kCut,       "a");
    TestShortcutString(Action::kCopy,      "b");
    TestShortcutString(Action::kDuplicate, "Ctrl-d");
    TestShortcutString(Action::kPaste,     "Shift-c");
    TestShortcutString(Action::kDelete,    "Shift-Ctrl-d");
    TestShortcutString(Action::kUndo,      "Shift-Ctrl-Alt-e");
    TestShortcutString(Action::kRedo,      "Shift-Ctrl-Alt-f");
    TestShortcutString(Action::kQuit,      "Shift-Ctrl-Alt-g");
}

TEST_F(ShortcutHandlerTest, CustomFromFile) {
    // Nonexistent file results in no error.
    EXPECT_TRUE(sh.AddCustomShortcutsFromFile("/NOSUCH/FILE.txt", error));

    // Use a FakeFileSystem to simulate a bad read of an existing file.
    GetFakeFileSystem()->AddFile("/NOSUCH/FILE.txt");
    EXPECT_FALSE(sh.AddCustomShortcutsFromFile("/NOSUCH/FILE.txt", error));
    TestErrorString("Unable to read");

    // Need a real FileSystem for the real test.
    UseRealFileSystem(true);

    const Str input =
        "Shift-4      Paste\n"
        "Ctrl-Shift-d Delete\n";

    {
        TempFile temp(input);
        EXPECT_TRUE(sh.AddCustomShortcutsFromFile(temp.GetPath(), error));
        EXPECT_TRUE(error.empty());
    }

    TestShortcutString(Action::kPaste,  "Shift-4");
    TestShortcutString(Action::kDelete, "Shift-Ctrl-d");
}

TEST_F(ShortcutHandlerTest, CustomBadSyntax) {
    EXPECT_FALSE(AddInput("blah\n"));
    TestErrorString("Syntax");
    TestErrorLine(1);

    EXPECT_FALSE(AddInput("a Cut\n"
                          "b Copy x"));
    TestErrorString("Syntax");
    TestErrorLine(2);
}

TEST_F(ShortcutHandlerTest, CustomBadModifier) {
    EXPECT_FALSE(AddInput("Alto-c Cut\n"));
    TestErrorString("Invalid key modifier");
    TestErrorLine(1);

    EXPECT_FALSE(AddInput("Alt-Ctrlf-c Cut\n"));
    TestErrorString("Invalid key modifier");
    TestErrorLine(1);
}

TEST_F(ShortcutHandlerTest, CustomBadAction) {
    EXPECT_FALSE(AddInput("Ctrl-c Cute\n"));
    TestErrorString("Invalid action");
    TestErrorLine(1);
}
