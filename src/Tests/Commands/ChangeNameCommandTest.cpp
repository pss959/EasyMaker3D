#include "Commands/ChangeNameCommand.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeNameCommandTest : public CommandTestBase {
  protected:
    ChangeNameCommandTest() { SetParseTypeName("ChangeNameCommand"); }
};

TEST_F(ChangeNameCommandTest, Default) {
    auto cnc = Command::CreateCommand<ChangeNameCommand>();
    EXPECT_TRUE(cnc->GetNewName().empty());
}

TEST_F(ChangeNameCommandTest, Set) {
    auto cnc = Command::CreateCommand<ChangeNameCommand>();
    cnc->SetNewName("Joe 51");
    EXPECT_EQ("Joe 51", cnc->GetNewName());
}

TEST_F(ChangeNameCommandTest, IsValid) {
    TestInvalid("",                          "Invalid model name");
    TestInvalid(R"(model_name: " BadName")", "Invalid model name");
    TestInvalid(R"(model_name: "Imp")",      "Invalid new model name");
    TestInvalid(R"(model_name: "Imp", new_name: "Blah ")",
                "Invalid new model name");
    TestValid(R"(model_name: "Imp", new_name: "Archie")");
}

TEST_F(ChangeNameCommandTest, GetDescription) {
    TestDesc(R"(model_name: "Box", new_name: "Blah")",
             R"(Changed the name of Model "Box")");
}
