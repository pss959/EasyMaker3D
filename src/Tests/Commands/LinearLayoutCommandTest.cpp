#include "Commands/LinearLayoutCommand.h"
#include "Place/EdgeTarget.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class LinearLayoutCommandTest : public CommandTestBase {
  protected:
    LinearLayoutCommandTest() { SetParseTypeName("LinearLayoutCommand"); }
};

TEST_F(LinearLayoutCommandTest, Default) {
    auto llc = Command::CreateCommand<LinearLayoutCommand>();
    EXPECT_EQ(Vector3f::Zero(), llc->GetOffset());
}

TEST_F(LinearLayoutCommandTest, Set) {
    // Set up an EdgeTarget for setting values.
    auto et = CreateObject<EdgeTarget>();
    et->SetPositions(Point3f(1, -2, 3), Point3f(1,  2, 3));

    auto llc = Command::CreateCommand<LinearLayoutCommand>();
    llc->SetFromTarget(*et);
    EXPECT_EQ(Vector3f(0, 4, 0), llc->GetOffset());
}

TEST_F(LinearLayoutCommandTest, IsValid) {
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [ " BadName" ])", "Invalid model name");
    TestValid(R"(model_names: ["Box"], offset: 1 2 -3)");
}

TEST_F(LinearLayoutCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Box"])",   R"(Linear layout of Model "Box")");
    TestDesc(R"(model_names: ["A", "B"])",  "Linear layout of 2 Models");
}
