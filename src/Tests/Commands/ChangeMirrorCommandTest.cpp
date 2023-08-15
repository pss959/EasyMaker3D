#include "Commands/ChangeMirrorCommand.h"
#include "Math/Types.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeMirrorCommandTest : public CommandTestBase {
  protected:
    ChangeMirrorCommandTest() { SetParseTypeName("ChangeMirrorCommand"); }
};

TEST_F(ChangeMirrorCommandTest, Default) {
    auto cmc = Command::CreateCommand<ChangeMirrorCommand>();
    EXPECT_EQ(Plane(), cmc->GetPlane());
}

TEST_F(ChangeMirrorCommandTest, Set) {
    auto cmc = Command::CreateCommand<ChangeMirrorCommand>();
    const Plane plane(Point3f(1, 2, 3), Vector3f(0, 0, 1));
    cmc->SetPlane(plane);
    EXPECT_EQ(plane, cmc->GetPlane());
}

TEST_F(ChangeMirrorCommandTest, IsValid) {
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [" BadName"])", "Invalid model name");
    TestInvalid(R"(model_names: ["X"], plane: 0 0 0 10)",
                "Zero-length plane normal");
    TestValid(R"(model_names: ["X"], plane: 1 0 0 2)");
}

TEST_F(ChangeMirrorCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Box"])",
             R"(Mirrored Model "Box" across plane )");
    TestDesc(R"(model_names: ["A", "B"])",
             "Mirrored 2 Models across plane ");
}
