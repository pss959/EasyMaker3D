#include "Panes/SpecialKeyPane.h"
#include "Panes/TextKeyPane.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"

/// KeyPane is an abstract class - this is used to test the derived TextKeyPane
/// and SpecialKeyPane classes.
///
/// \ingroup Tests
class KeyPaneTest : public PaneTestBase {};

// ----------------------------------------------------------------------------
// TextKeyPane tests.
// ----------------------------------------------------------------------------

TEST_F(KeyPaneTest, TextKeyPaneIsValid) {
    SetParseTypeName("TextKeyPane");
    TestInvalid("",             "No text specified");
    TestInvalid(R"(text: "a")", "No shifted text specified");
}

TEST_F(KeyPaneTest, TextKeyPaneAction) {
    auto tkey = ReadRealPane<TextKeyPane>("TextKeyPane",
                                          R"(text: "a", shifted_text: "A")");
    Str chars;
    EXPECT_EQ(TextAction::kInsert, tkey->GetAction(chars));
    EXPECT_EQ("a", chars);

    tkey->ProcessShift(true);
    EXPECT_EQ(TextAction::kInsert, tkey->GetAction(chars));
    EXPECT_EQ("A", chars);

    tkey->ProcessShift(false);
    EXPECT_EQ(TextAction::kInsert, tkey->GetAction(chars));
    EXPECT_EQ("a", chars);
}

TEST_F(KeyPaneTest, TextKeyPaneActionWithLabel) {
    auto tkey = ReadRealPane<TextKeyPane>(
        "TextKeyPane", R"(text: "b", shifted_text: "B", label: "Pop")");
    Str chars;
    EXPECT_EQ(TextAction::kInsert, tkey->GetAction(chars));
    EXPECT_EQ("Pop", chars);

    tkey->ProcessShift(true);
    EXPECT_EQ(TextAction::kInsert, tkey->GetAction(chars));
    EXPECT_EQ("Pop", chars);

    tkey->ProcessShift(false);
    EXPECT_EQ(TextAction::kInsert, tkey->GetAction(chars));
    EXPECT_EQ("Pop", chars);
}

// ----------------------------------------------------------------------------
// SpecialKeyPane tests.
// ----------------------------------------------------------------------------

TEST_F(KeyPaneTest, SpecialKeyPaneIsValid) {
    SetParseTypeName("SpecialKeyPane");
    TestInvalid("", "No icon_name or label specified");
    TestInvalid(R"(label: "a", action: "kInsert")",
                "SpecialKeyPane action should not be kInsert");
}

TEST_F(KeyPaneTest, SpecialKeyPaneActionWithLabel) {
    auto tkey = ReadRealPane<SpecialKeyPane>(
        "SpecialKeyPane", R"(label: "Undo", action: "kUndo")");
    Str chars;
    EXPECT_EQ(TextAction::kUndo, tkey->GetAction(chars));
    EXPECT_TRUE(chars.empty());

    // ProcessShift has no effect.
    tkey->ProcessShift(true);
    EXPECT_EQ(TextAction::kUndo, tkey->GetAction(chars));
    EXPECT_TRUE(chars.empty());
}

TEST_F(KeyPaneTest, SpecialKeyPaneActionWithIcon) {
    auto tkey = ReadRealPane<SpecialKeyPane>(
        "SpecialKeyPane", R"(icon_name: "ForwardIcon", action: "kMoveNext")");
    Str chars;
    EXPECT_EQ(TextAction::kMoveNext, tkey->GetAction(chars));
    EXPECT_TRUE(chars.empty());

    // ProcessShift has no effect.
    tkey->ProcessShift(true);
    EXPECT_EQ(TextAction::kMoveNext, tkey->GetAction(chars));
    EXPECT_TRUE(chars.empty());
}
