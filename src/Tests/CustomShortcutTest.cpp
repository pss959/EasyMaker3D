#include "Handlers/ShortcutHandler.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"
#include "Util/String.h"

class CustomShortcutTest : public TestBase {
  protected:
    // ShortcutHandler and error string common to all tests.
    ShortcutHandler sh;
    std::string     error;

    bool AddInput(const std::string &input) {
        return sh.AddCustomShortcutsFromString(input, error);
    }

    void TestErrorString(const std::string &pattern) {
        TestString(error, pattern);
    }

    void TestErrorLine(int line) {
        TestString(error, "line " + Util::ToString(line));
    }

    void TestShortcutString(Action action, const std::string &pattern) {
        std::string ks, cs;
        sh.GetShortcutStrings(action, ks, cs);
        TestString(ks, pattern);
    }

    void TestString(const std::string &s, const std::string &pattern) {
        EXPECT_TRUE(Util::StringContains(s, pattern));
    }
};


TEST_F(CustomShortcutTest, Empty) {
    EXPECT_TRUE(AddInput(""));
    EXPECT_TRUE(error.empty());
}

TEST_F(CustomShortcutTest, Comments) {
    const std::string input =
        "# Hello there\n"
        "     # Otherwise blank   \n"
        "##############\n";
    EXPECT_TRUE(AddInput(input));
    EXPECT_TRUE(error.empty());
}

TEST_F(CustomShortcutTest, Valid) {
    const std::string input =
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

    TestShortcutString(Action::kCut, "a");
    TestShortcutString(Action::kCopy, "b");
    TestShortcutString(Action::kPaste, "Shift-c");
    TestShortcutString(Action::kDelete, "Shift-Ctrl-d");
    TestShortcutString(Action::kUndo, "Shift-Ctrl-Alt-e");
    TestShortcutString(Action::kRedo, "Shift-Ctrl-Alt-f");
    TestShortcutString(Action::kQuit, "Shift-Ctrl-Alt-g");
}

TEST_F(CustomShortcutTest, BadSyntax) {
    EXPECT_FALSE(AddInput("blah\n"));
    TestErrorString("Syntax");
    TestErrorLine(1);

    EXPECT_FALSE(AddInput("a Cut\n"
                          "b Copy x"));
    TestErrorString("Syntax");
    TestErrorLine(2);
}

TEST_F(CustomShortcutTest, BadModifier) {
    EXPECT_FALSE(AddInput("Alto-c Cut\n"));
    TestErrorString("Invalid key modifier");
    TestErrorLine(1);

    EXPECT_FALSE(AddInput("Alt-Ctrlf-c Cut\n"));
    TestErrorString("Invalid key modifier");
    TestErrorLine(1);
}

TEST_F(CustomShortcutTest, BadAction) {
    EXPECT_FALSE(AddInput("Ctrl-c Cute\n"));
    TestErrorString("Invalid action");
    TestErrorLine(1);
}
