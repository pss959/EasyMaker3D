#include "Commands/ChangeTwistCommand.h"
#include "Math/Spin.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeTwistCommandTest : public CommandTestBase {
  protected:
    ChangeTwistCommandTest() { SetParseTypeName("ChangeTwistCommand"); }
};

TEST_F(ChangeTwistCommandTest, Default) {
    auto ctc = Command::CreateCommand<ChangeTwistCommand>();
    EXPECT_EQ(Spin(), ctc->GetSpin());
}

TEST_F(ChangeTwistCommandTest, Set) {
    Spin spin;
    spin.center.Set(2, 3, 4);
    spin.axis.Set(0, 0, 1);
    spin.angle = Anglef::FromDegrees(45);
    spin.offset = 3;

    auto ctc = Command::CreateCommand<ChangeTwistCommand>();
    ctc->SetSpin(spin);
    EXPECT_EQ(spin, ctc->GetSpin());
}

TEST_F(ChangeTwistCommandTest, IsValid) {
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [ " BadName" ])", "Invalid model name");
    TestInvalid(R"(model_names: ["X"], axis: 0 0 0)",
                "Zero-length spin axis vector");
    TestValid(R"(model_names: ["Box"])");
}

TEST_F(ChangeTwistCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Box"])",    R"(Twisted Model "Box")");
    TestDesc(R"(model_names: ["A", "B"])",   "Twisted 2 Models");
}
