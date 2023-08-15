#include "Commands/ChangeRevSurfCommand.h"
#include "Models/RevSurfModel.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeRevSurfCommandTest : public CommandTestBase {
  protected:
  protected:
    ChangeRevSurfCommandTest() { SetParseTypeName("ChangeRevSurfCommand"); }

    /// Convenient string with valid profile points.
    const std::string ppts = "profile_points: [.6 .5, .4 .2]";
};

TEST_F(ChangeRevSurfCommandTest, Default) {
    auto crsc = Command::CreateCommand<ChangeRevSurfCommand>();
    EXPECT_FALSE(crsc->GetProfile().IsValid());
    EXPECT_EQ(360, crsc->GetSweepAngle().Degrees());
}

TEST_F(ChangeRevSurfCommandTest, Set) {
    auto crsc = Command::CreateCommand<ChangeRevSurfCommand>();
    const Profile prof = RevSurfModel::CreateDefaultProfile();
    crsc->SetProfile(prof);
    crsc->SetSweepAngle(Anglef::FromDegrees(24));
    EXPECT_EQ(prof, crsc->GetProfile());
    EXPECT_EQ(24,   crsc->GetSweepAngle().Degrees());
}

TEST_F(ChangeRevSurfCommandTest, IsValid) {
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [" BadName"])", "Invalid model name");
    TestInvalid(R"(model_names: ["Rev"])", "Invalid profile");
    TestInvalid(R"(model_names: ["Rev"], )" + ppts + ", sweep_angle: 0",
                "Sweep angle is out of range");
    TestInvalid(R"(model_names: ["Rev"], )" + ppts + ", sweep_angle: -10",
                "Sweep angle is out of range");
    TestInvalid(R"(model_names: ["Rev"], )" + ppts + ", sweep_angle: 361",
                "Sweep angle is out of range");
    TestValid(R"(model_names: ["Rev"], )" + ppts);
}

TEST_F(ChangeRevSurfCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Rev"], )" + ppts,
             R"(Changed the profile or sweep angle in Model "Rev")");
    TestDesc(R"(model_names: ["Rev0", "Rev1"], )" + ppts,
             "Changed the profile or sweep angle in 2 Models");
}
