#include "Commands/ChangeBevelCommand.h"
#include "Math/Types.h"
#include "Models/BeveledModel.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeBevelCommandTest : public CommandTestBase {
  protected:
    ChangeBevelCommandTest() { SetParseTypeName("ChangeBevelCommand"); }
};

TEST_F(ChangeBevelCommandTest, Default) {
    auto cbc = Command::CreateCommand<ChangeBevelCommand>();
    EXPECT_EQ(Bevel(), cbc->GetBevel());
}

TEST_F(ChangeBevelCommandTest, Set) {
    Bevel bevel;
    bevel.profile =
        BeveledModel::CreateProfile(Profile::PointVec{ Point2f(.4f, .5f) });
    bevel.scale     = 2.4f;
    bevel.max_angle = Anglef::FromDegrees(90);

    auto cbc = Command::CreateCommand<ChangeBevelCommand>();
    cbc->SetBevel(bevel);
    EXPECT_EQ(bevel, cbc->GetBevel());
}

TEST_F(ChangeBevelCommandTest, IsValid) {
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [" BadName"])", "Invalid model name");
    TestInvalid(R"(model_names: ["X"], profile_points: [2 1])",
                "Invalid profile");
    TestInvalid(R"(model_names: ["X"], bevel_scale: 0)",
                "Bevel scale is not positive");
    TestInvalid(R"(model_names: ["X"], max_angle: -1)",
                "Maximum angle is out of range");
    TestInvalid(R"(model_names: ["X"], max_angle: 181)",
                "Maximum angle is out of range");
    TestValid(R"(model_names: ["Box"])");
}

TEST_F(ChangeBevelCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Box"])", R"(Changed the bevel of Model "Box")");
    TestDesc(R"(model_names: ["A", "B"])", "Changed the bevel of 2 Models");
}
