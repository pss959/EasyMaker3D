#include "Commands/ChangeExtrudedCommand.h"
#include "Models/ExtrudedModel.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeExtrudedCommandTest : public CommandTestBase {
  protected:
    ChangeExtrudedCommandTest() { SetParseTypeName("ChangeExtrudedCommand"); }

    /// Convenient string with valid profile points.
    const Str ppts = "profile_points: [.2 .4, .8 .6, .4 .8]";
};

TEST_F(ChangeExtrudedCommandTest, Default) {
    auto cec = Command::CreateCommand<ChangeExtrudedCommand>();
    EXPECT_FALSE(cec->GetProfile().IsValid());
}

TEST_F(ChangeExtrudedCommandTest, Set) {
    auto cec = Command::CreateCommand<ChangeExtrudedCommand>();
    const Profile prof = ExtrudedModel::CreateRegularPolygonProfile(5);
    cec->SetProfile(prof);
    EXPECT_EQ(prof, cec->GetProfile());
    EXPECT_TRUE(cec->GetProfile().IsValid());
}

TEST_F(ChangeExtrudedCommandTest, IsValid) {
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [" BadName"])", "Invalid model name");
    TestInvalid(R"(model_names: [" BadName"])", "Invalid model name");
    TestInvalid(R"(model_names: ["Ext"])", "Invalid profile");
    TestValid(R"(model_names: ["Ext"], )" + ppts);
}

TEST_F(ChangeExtrudedCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Ext"], )" + ppts,
             R"(Changed the profile in Model "Ext")");
    TestDesc(R"(model_names: ["Ext0", "Ext1"], )" + ppts,
             "Changed the profile in 2 Models");
}
