#include "Commands/ChangeClipCommand.h"
#include "Math/Types.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeClipCommandTest : public CommandTestBase {
  protected:
    ChangeClipCommandTest() { SetParseTypeName("ChangeClipCommand"); }
};

TEST_F(ChangeClipCommandTest, Default) {
    auto ccc = Command::CreateCommand<ChangeClipCommand>();
    EXPECT_EQ(Plane(), ccc->GetPlane());
}

TEST_F(ChangeClipCommandTest, Set) {
    auto ccc = Command::CreateCommand<ChangeClipCommand>();
    const Plane plane(Point3f(1, 2, 3), Vector3f(0, 0, 1));
    ccc->SetPlane(plane);
    EXPECT_EQ(plane, ccc->GetPlane());
}

TEST_F(ChangeClipCommandTest, IsValid) {
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [" BadName"])", "Invalid model name");
    TestInvalid(R"(model_names: ["X"], plane: 0 0 0 10)",
                "Zero-length plane normal");
    TestValid(R"(model_names: ["X"], plane: 1 0 0 2)");
}

TEST_F(ChangeClipCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Box"])",
             R"(Changed the clip plane of Model "Box" to )");
    TestDesc(R"(model_names: ["A", "B"])",
             "Changed the clip plane of 2 Models to ");
}
