#include "Panes/TextPane.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"
#include "Tests/UnitTestTypeChanger.h"
#include "Util/Tuning.h"

/// \ingroup Tests
class TextPaneTest : public PaneTestBase {
  protected:
    TextPanePtr GetTextPane(const Str &contents = "") {
        return ReadRealPane<TextPane>("TextPane", contents);
    }
};

TEST_F(TextPaneTest, Defaults) {
    auto text = GetTextPane();
    EXPECT_EQ(".",              text->GetText());
    EXPECT_EQ(Color::Black(),   text->GetColor());
    EXPECT_EQ(TK::k3DFont,      text->GetFontName());
    EXPECT_EQ(12,               text->GetFontSize());
    EXPECT_EQ(1,                text->GetCharacterSpacing());
    EXPECT_EQ(Vector2f::Zero(), text->GetOffset());
    EXPECT_TRUE(text->IsTextResizable());
}

TEST_F(TextPaneTest, Set) {
    auto text = GetTextPane();
    text->SetText("Hello");
    text->SetColor(Color(1, 1, 0));
    text->SetFontName("Verdana-Regular");
    text->SetFontSize(13.5f);
    text->SetCharacterSpacing(1.2f);
    text->SetOffset(Vector2f(1, -1));
    text->SetTextResizable(false);

    EXPECT_EQ("Hello",           text->GetText());
    EXPECT_EQ(Color(1, 1, 0),    text->GetColor());
    EXPECT_EQ("Verdana-Regular", text->GetFontName());
    EXPECT_EQ(13.5f,             text->GetFontSize());
    EXPECT_EQ(1.2f,              text->GetCharacterSpacing());
    EXPECT_EQ(Vector2f(1, -1),   text->GetOffset());
    EXPECT_FALSE(text->IsTextResizable());

    // Setting text to empty is legal: it should disable the TextNode.
    text->SetText("");
    EXPECT_EQ("", text->GetText());
}

TEST_F(TextPaneTest, Text) {
    // Override this setting for this test; need to build font images.
    UnitTestTypeChanger uttc(Util::AppType::kInteractive);

    auto text = GetTextPane();

    text->SetText("ABC");
    text->SetFontSize(22);
    EXPECT_VECS_CLOSE2(Vector2f(66, 22), text->GetBaseSize());

    // Try multi-line text.
    text->SetText("ABC\nDE");
    EXPECT_VECS_CLOSE2(Vector2f(264, 44), text->GetBaseSize());

    text->SetFontSize(44);
    EXPECT_VECS_CLOSE2(Vector2f(528, 88), text->GetBaseSize());

    // Repeat with resizable height.
    text->SetHeightResizable(true);
    EXPECT_VECS_CLOSE2(Vector2f(528, 88), text->GetBaseSize());

    text->SetLayoutSize(Vector2f(100, 60));
    EXPECT_VECS_CLOSE2(Vector2f(100, 60), text->GetLayoutSize());

    // Repeat with smaller aspect ratio.
    text->SetLayoutSize(Vector2f(60, 100));
    EXPECT_VECS_CLOSE2(Vector2f(528, 88), text->GetBaseSize());

    EXPECT_VECS_CLOSE2(Vector2f(60, 10), text->GetTextSize());

    // Change the text and make sure the base size updates.
    text->SetText("abcdefgh");
    EXPECT_VECS_CLOSE2(Vector2f(352, 44), text->GetBaseSize());

    // Repeat with fixed height.
    text->SetHeightResizable(false);
    text->SetLayoutSize(Vector2f(200, 60));
    EXPECT_VECS_CLOSE2(Vector2f(352, 44), text->GetBaseSize());
}

TEST_F(TextPaneTest, IsValid) {
    // Override this setting for this test; need to build font images.
    UnitTestTypeChanger uttc(Util::AppType::kInteractive);

    SetParseTypeName("TextPane");
    TestInvalid(R"(text: "")", "Empty text string");
}

TEST_F(TextPaneTest, Notify) {
    auto text = GetTextPane();

    size_t change_count = 0;
    text->GetChanged().AddObserver(
        "key", [&](SG::Change, const SG::Object &){ ++change_count; });

    EXPECT_EQ(0U, change_count);
    text->SetUniformScale(2);
    EXPECT_EQ(1U, change_count);

    text->SetNotifyEnabled(false);
    text->SetUniformScale(3);
    EXPECT_EQ(1U, change_count);
}
