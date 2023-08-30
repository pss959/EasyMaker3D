#include "Commands/ChangeTextCommand.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"
#include "Tests/UnitTestTypeChanger.h"

/// \ingroup Tests
class ChangeTextCommandTest : public CommandTestBase {
  protected:
    ChangeTextCommandTest() { SetParseTypeName("ChangeTextCommand"); }
};

TEST_F(ChangeTextCommandTest, Default) {
    auto ctc = Command::CreateCommand<ChangeTextCommand>();
    EXPECT_TRUE(ctc->GetNewTextString().empty());
    EXPECT_TRUE(ctc->GetNewFontName().empty());
    EXPECT_EQ(1, ctc->GetNewCharSpacing());
}

TEST_F(ChangeTextCommandTest, Set) {
    // This is necessary to get the font set up so the name is recognized.
    UnitTestTypeChanger uttc(Util::AppType::kInteractive);
    auto ctc = Command::CreateCommand<ChangeTextCommand>();
    ctc->SetNewTextString("Hi there!");
    ctc->SetNewFontName("FontA");
    ctc->SetNewCharSpacing(2.1f);
    EXPECT_EQ("Hi there!", ctc->GetNewTextString());
    EXPECT_EQ("FontA",     ctc->GetNewFontName());
    EXPECT_EQ(2.1f,        ctc->GetNewCharSpacing());
}

TEST_F(ChangeTextCommandTest, IsValid) {
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [" BadName"])", "Invalid model name");
    TestInvalid(R"(model_names: ["Text"], new_font_name: "Blah")",
                "Invalid new font name");
    TestInvalid(R"(model_names: ["Text"], new_text_string: "")",
                "Empty new text string");
    TestValid(R"(model_names: ["Text"])");
}

TEST_F(ChangeTextCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Text"])",
             R"(Changed the text or options in Model "Text")");
    TestDesc(R"(model_names: ["A", "B"])",
             "Changed the text or options in 2 Models");
}
