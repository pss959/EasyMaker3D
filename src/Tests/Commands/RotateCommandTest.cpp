#include "Commands/RotateCommand.h"
#include "Math/Types.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class RotateCommandTest : public CommandTestBase {
  protected:
    RotateCommandTest() { SetParseTypeName("RotateCommand"); }
};

TEST_F(RotateCommandTest, Default) {
    auto rc = Command::CreateCommand<RotateCommand>();
    EXPECT_TRUE(rc->GetModelNames().empty());
    EXPECT_EQ(Rotationf::Identity(), rc->GetRotation());
    EXPECT_FALSE(rc->IsInPlace());
    EXPECT_FALSE(rc->IsAxisAligned());
}

TEST_F(RotateCommandTest, Set) {
    auto rc = Command::CreateCommand<RotateCommand>();
    const Rotationf rot = BuildRotation(0, 0, 1, -270);
    rc->SetRotation(rot);
    rc->SetIsInPlace(true);
    rc->SetIsAxisAligned(true);
    EXPECT_EQ(rot, rc->GetRotation());
    EXPECT_TRUE(rc->IsInPlace());
    EXPECT_TRUE(rc->IsAxisAligned());
}

TEST_F(RotateCommandTest, IsValid) {
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [ " BadName" ])", "Invalid model name");
    TestValid(R"(model_names: ["Box"], rotation: 0 1 0 20)");
}

TEST_F(RotateCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Box"], rotation: 0 0 1 90)",
             R"(Rotated Model "Box")");
    TestDesc(R"(model_names: ["A", "B"], is_in_place: True)",
             "Rotated 2 Models (in place)");
}
