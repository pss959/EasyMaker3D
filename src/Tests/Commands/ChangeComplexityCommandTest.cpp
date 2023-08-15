#include "Commands/ChangeComplexityCommand.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeComplexityCommandTest : public CommandTestBase {
  protected:
    ChangeComplexityCommandTest() {
        SetParseTypeName("ChangeComplexityCommand");
    }
};

TEST_F(ChangeComplexityCommandTest, Default) {
    auto ccc = Command::CreateCommand<ChangeComplexityCommand>();
    EXPECT_EQ(0, ccc->GetNewComplexity());
}

TEST_F(ChangeComplexityCommandTest, Set) {
    auto ccc = Command::CreateCommand<ChangeComplexityCommand>();
    ccc->SetNewComplexity(.8f);
    EXPECT_EQ(.8f, ccc->GetNewComplexity());
}

TEST_F(ChangeComplexityCommandTest, IsValid) {
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [" BadName"])", "Invalid model name");
    TestValid(R"(model_names: ["Box"])");
}

TEST_F(ChangeComplexityCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Box"])",
             R"(Changed the complexity of Model "Box")");
    TestDesc(R"(model_names: ["A", "B"])",
             "Changed the complexity of 2 Models");
}
