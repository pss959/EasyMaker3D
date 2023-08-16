#include "Commands/TranslateCommand.h"
#include "Math/Types.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class TranslateCommandTest : public CommandTestBase {
  protected:
    TranslateCommandTest() { SetParseTypeName("TranslateCommand"); }
};

TEST_F(TranslateCommandTest, Default) {
    auto tc = Command::CreateCommand<TranslateCommand>();
    EXPECT_EQ(Vector3f(0, 0, 0), tc->GetTranslation());
    EXPECT_TRUE(tc->GetModelNames().empty());
}

TEST_F(TranslateCommandTest, Set) {
    auto tc = Command::CreateCommand<TranslateCommand>();
    tc->SetTranslation(Vector3f(2, 3, 4));
    EXPECT_EQ(Vector3f(2, 3, 4), tc->GetTranslation());
}

TEST_F(TranslateCommandTest, IsValid) {
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [ " BadName" ])", "Invalid model name");
    TestValid(R"(model_names: ["Box"], translation: -1 1 2)");
}

TEST_F(TranslateCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Box"], translation: 1 2 3)",
             R"(Translated Model "Box")");
    TestDesc(R"(model_names: ["A", "B"])", "Translated 2 Models");
}
