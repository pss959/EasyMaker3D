#include <vector>

#include <ion/base/logchecker.h>
#include <ion/gfx/shaderprogram.h>

#include "Math/TextUtils.h"
#include "SG/Exception.h"
#include "SG/LayoutOptions.h"
#include "SG/TextNode.h"
#include "Tests/SceneTestBase.h"
#include "Tests/TempFile.h"
#include "Tests/Testing.h"
#include "Tests/UnitTestTypeChanger.h"
#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/Tuning.h"

/// \ingroup Tests
class TextTest : public SceneTestBase {};

TEST_F(TextTest, DefaultTextNode) {
    auto text = CreateObject<SG::TextNode>();
    EXPECT_NOT_NULL(text);
    EXPECT_EQ(".",            text->GetText());
    EXPECT_EQ(TK::k3DFont,    text->GetFontName());
    EXPECT_EQ(32U,            text->GetFontSize());
    EXPECT_EQ(8U,             text->GetSDFPadding());
    EXPECT_EQ(512U,           text->GetMaxImageSize());
    EXPECT_EQ(Color::White(), text->GetColor());
    EXPECT_EQ(Color::Black(), text->GetOutlineColor());
    EXPECT_EQ(2,              text->GetOutlineWidth());
    EXPECT_EQ(3,              text->GetHalfSmoothWidth());
    EXPECT_NULL(text->GetLayoutOptions());

    SetUpIonForNode(*text);
    EXPECT_NOT_NULL(text->GetIonNode().Get());

    // No font image set up in regular unit tests.
    TEST_THROW(text->GetLineSpacingFactor(), AssertException, "font_image");

    // No size until set up with a font.
    EXPECT_EQ(Vector3f::Zero(), text->GetTextBounds().GetSize());
    EXPECT_EQ(Vector2f::Zero(), text->GetTextSize());
}

TEST_F(TextTest, SetUpFont) {
    // Use this to make sure the font is set up.
    UnitTestTypeChanger uttc(Util::AppType::kInteractive);

    auto text = CreateObject<SG::TextNode>();
    text->SetLayoutOptions(CreateObject<SG::LayoutOptions>());

    // No sizes or font image until SetUpIon() is called.
    EXPECT_EQ(Vector3f::Zero(), text->GetTextBounds().GetSize());
    EXPECT_EQ(Vector2f::Zero(), text->GetTextSize());
    TEST_THROW(text->GetLineSpacingFactor(), AssertException, "font_image");

    SetUpIonForNode(*text);

    EXPECT_VECS_CLOSE(Vector3f(.65625f, .59375f, 0),
                      text->GetTextBounds().GetSize());
    EXPECT_VECS_CLOSE2(Vector2f(.21875f, .09375f), text->GetTextSize());
    EXPECT_CLOSE(1.15625f, text->GetLineSpacingFactor());

    text->SetFontName("Arial-Bold");
    EXPECT_VECS_CLOSE(Vector3f(.65625f, .625f, 0),
                      text->GetTextBounds().GetSize());
    EXPECT_VECS_CLOSE2(Vector2f(.21875f, .125f), text->GetTextSize());
    EXPECT_CLOSE(1.15625f, text->GetLineSpacingFactor());

    // Changing the text should update the sizes.
    text->SetText("X");
    EXPECT_VECS_CLOSE(Vector3f(1.1875f, 1.21875f, 0),
                      text->GetTextBounds().GetSize());
    EXPECT_VECS_CLOSE2(Vector2f(.6875f, .71875f), text->GetTextSize());
    EXPECT_CLOSE(1.15625f, text->GetLineSpacingFactor());

    // Turn off notification, should not update when changing LayoutOptions.
    text->SetNotifyEnabled(false);
    text->GetLayoutOptions()->SetHAlignment(
        SG::LayoutOptions::HAlignment::kAlignHCenter);
    EXPECT_VECS_CLOSE2(Vector2f(.6875f, .71875f), text->GetTextSize());

    // Re-enable notification and change the text, but force a rebuild from
    // GetTextSize().
    text->SetNotifyEnabled(true);
    text->SetText("XX");
    EXPECT_VECS_CLOSE2(Vector2f(1.34375f, .71875f), text->GetTextSize());
}

TEST_F(TextTest, ChangeTextNode) {
    auto text = CreateObject<SG::TextNode>();

    text->SetFontName("SomeFont");
    text->SetFontSize(22);
    text->SetText("New text");
    text->SetTextColor(Color(1, 0, 0));
    EXPECT_EQ("New text",     text->GetText());
    EXPECT_EQ("SomeFont",     text->GetFontName());
    EXPECT_EQ(22U,            text->GetFontSize());
    EXPECT_EQ(Color(1, 0, 0), text->GetColor());

    text->SetTextWithColor("Newer text", Color(0, 1, 0));
    EXPECT_EQ("Newer text",   text->GetText());
    EXPECT_EQ("SomeFont",     text->GetFontName());
    EXPECT_EQ(22U,            text->GetFontSize());
    EXPECT_EQ(Color(0, 1, 0), text->GetColor());

    text->SetWorldScaleAndRotation(Matrix4f::Identity(), 2,
                                   BuildRotation(0, 1, 0, 90));
    EXPECT_VECS_CLOSE(Vector3f(2, 2, 2),           text->GetScale());
    EXPECT_ROTS_CLOSE(BuildRotation(0, -1, 0, 90), text->GetRotation());
}

TEST_F(TextTest, LayoutOptions) {
    // Shorthand. (Can't use "using" because they are not enum classes.)
    typedef SG::LayoutOptions::HAlignment HAlignment;
    typedef SG::LayoutOptions::VAlignment VAlignment;

    // Use this to make sure the font is set up.
    UnitTestTypeChanger uttc(Util::AppType::kInteractive);

    // Set up LayoutOptions.
    auto layout = CreateObject<SG::LayoutOptions>();
    EXPECT_EQ(HAlignment::kAlignLeft,     layout->GetHAlignment());
    EXPECT_EQ(VAlignment::kAlignBaseline, layout->GetVAlignment());
    EXPECT_EQ(1,                          layout->GetLineSpacing());
    EXPECT_EQ(0,                          layout->GetGlyphSpacing());
    EXPECT_FALSE(layout->IsUsingMetricsBasedAlignment());

    layout->SetHAlignment(HAlignment::kAlignRight);
    layout->SetVAlignment(VAlignment::kAlignTop);
    layout->SetLineSpacing(2);
    layout->SetGlyphSpacing(.2f);
    EXPECT_EQ(HAlignment::kAlignRight, layout->GetHAlignment());
    EXPECT_EQ(VAlignment::kAlignTop,   layout->GetVAlignment());
    EXPECT_EQ(2,                       layout->GetLineSpacing());
    EXPECT_EQ(.2f,                     layout->GetGlyphSpacing());
    EXPECT_FALSE(layout->IsUsingMetricsBasedAlignment());

    auto text = CreateObject<SG::TextNode>();
    text->SetLayoutOptions(layout);

    SetUpIonForNode(*text);
}

TEST_F(TextTest, InvalidTextNode) {
    TestInvalid(R"(TextNode { text: "" })", "Empty text string");
}

TEST_F(TextTest, BadText) {
    // Use this to make sure the font is set up.
    UnitTestTypeChanger uttc(Util::AppType::kInteractive);

    auto text = CreateObject<SG::TextNode>();

    // Set the text to not have any valid glyphs.
    text->SetText("\t");

    // Force a build.
    TEST_THROW(SetUpIonForNode(*text), SG::Exception,
               "Unable to build Ion text");
}

TEST_F(TextTest, BadFont) {
    // Set up a temporary font file with nothing in it. Add it to the font
    // manager while this is still considered a unit test.
    TempFile tmp("");
    AddFontPath("BadFont", tmp.GetPath());

    // Use this to make sure the font is set up.
    UnitTestTypeChanger uttc(Util::AppType::kInteractive);

    auto text = CreateObject<SG::TextNode>();
    text->SetText("A");

    // Use a nonexistent font name and force a build.
    text->SetFontName("NoSuchFont");
    TEST_THROW(SetUpIonForNode(*text), SG::Exception, "does not exist");

    {
        // Use the bad font, forcing a rebuild. This test generates an Ion
        // error.
        ion::base::LogChecker log_checker;
        TEST_THROW(text->SetFontName("BadFont"), SG::Exception,
                   "Unable to create font");
        EXPECT_TRUE(log_checker.GetLogString().contains("Unable to read data"));
        log_checker.ClearLog();
    }

    // Create a TextNode with a very small maximum image size.
    auto text2 = ParseTypedObject<SG::TextNode>(
        "TextNode { max_image_size: 4 }");
    TEST_THROW(SetUpIonForNode(*text2), SG::Exception,
               "Unable to create font image");
}
