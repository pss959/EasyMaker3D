#include "Math/Profile.h"
#include "Models/TextModel.h"
#include "Tests/Testing.h"
#include "Tests/SceneTestBase.h"
#include "Util/Tuning.h"

/// \ingroup Tests
class TextModelTest : public SceneTestBase {};

TEST_F(TextModelTest, Defaults) {
    auto text = Model::CreateModel<TextModel>();

    EXPECT_TRUE(text->CanSetComplexity());

    EXPECT_TRUE(text->GetTextString().empty());
    EXPECT_EQ(TK::k3DFont, text->GetFontName());
    EXPECT_EQ(1,           text->GetCharSpacing());
}

TEST_F(TextModelTest, Set) {
    auto text = Model::CreateModel<TextModel>();

    const Str font = "DejaVu Sans Mono-Book";
    text->SetTextString("Hello");
    text->SetFontName(font);
    text->SetCharSpacing(1.5f);

    EXPECT_EQ("Hello", text->GetTextString());
    EXPECT_EQ(font,    text->GetFontName());
    EXPECT_EQ(1.5f,    text->GetCharSpacing());
}

TEST_F(TextModelTest, IsValid) {
    SetParseTypeName("TextModel");
    TestInvalid(R"(font_name: "Blah")", "Unknown font name");
    TestInvalid("",                     "Empty text string");
    TestValid(R"(text: "Something")");
}

TEST_F(TextModelTest, Mesh) {
    // Need a real set-up TextModel for GetMesh() to work.
    auto text = ReadRealNode<TextModel>(
        R"(children: [ TextModel "TestText" { text: "HI" } ])", "TestText");

    const auto mesh = text->GetMesh();
    EXPECT_EQ(32U, mesh.points.size());
    EXPECT_EQ(56U, mesh.GetTriangleCount());
    EXPECT_EQ(Vector3f::Zero(), text->GetObjectCenterOffset());
}

