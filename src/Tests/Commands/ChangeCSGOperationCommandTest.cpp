#include "Commands/ChangeCSGOperationCommand.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeCSGOperationCommandTest : public CommandTestBase {};

TEST_F(ChangeCSGOperationCommandTest, Default) {
    auto ccc = Command::CreateCommand<ChangeCSGOperationCommand>();
    EXPECT_ENUM_EQ(CSGOperation::kUnion, ccc->GetNewOperation());
    EXPECT_TRUE(ccc->GetResultNames().empty());
}

TEST_F(ChangeCSGOperationCommandTest, Set) {
    auto ccc = Command::CreateCommand<ChangeCSGOperationCommand>();
    ccc->SetNewOperation(CSGOperation::kDifference);
    ccc->SetResultNames(std::vector<std::string>{ "Abc", "Def" });
    EXPECT_ENUM_EQ(CSGOperation::kDifference, ccc->GetNewOperation());
    EXPECT_EQ(2U, ccc->GetResultNames().size());
    EXPECT_EQ("Abc", ccc->GetResultNames()[0]);
    EXPECT_EQ("Def", ccc->GetResultNames()[1]);
}

TEST_F(ChangeCSGOperationCommandTest, IsValid) {
    TestInvalid("ChangeCSGOperationCommand {}", "Missing model names");
    TestInvalid(R"(ChangeCSGOperationCommand { model_names: [" BadName"] })",
                "Invalid model name");
    TestInvalid(R"(ChangeCSGOperationCommand { model_names: ["X"] })",
                "Missing result names");
    TestInvalid(R"(ChangeCSGOperationCommand { model_names: ["X"],
                    result_names: [" BadName "] })",
                "Invalid result model name");
    TestValid(R"(ChangeCSGOperationCommand { model_names: ["A", "B"],
                     result_names: ["C", "D"] })");
}

TEST_F(ChangeCSGOperationCommandTest, GetDescription) {
    TestDesc(R"(ChangeCSGOperationCommand { model_names: ["Box"],
                    result_names: ["X"] })",
             R"(Changed the CSG operation of Model "Box" to Union)");
    TestDesc(R"(ChangeCSGOperationCommand { model_names: ["A", "B"],
                    result_names: ["X", "Y"] })",
             "Changed the CSG operation of 2 Models to Union");
}
