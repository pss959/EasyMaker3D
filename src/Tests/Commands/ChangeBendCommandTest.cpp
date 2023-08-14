#include "Commands/ChangeBendCommand.h"
#include "Math/Types.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeBendCommandTest : public SceneTestBase {};

TEST_F(ChangeBendCommandTest, Default) {
    auto cbc = Command::CreateCommand<ChangeBendCommand>();
    EXPECT_EQ(Spin(), cbc->GetSpin());
}

TEST_F(ChangeBendCommandTest, Set) {
    Spin spin;
    spin.center.Set(2, 3, 4);
    spin.axis.Set(0, 0, 1);
    spin.angle = Anglef::FromDegrees(45);
    spin.offset = 3;

    auto cbc = Command::CreateCommand<ChangeBendCommand>();
    cbc->SetSpin(spin);
    EXPECT_EQ(spin, cbc->GetSpin());
}

TEST_F(ChangeBendCommandTest, IsValid) {
    TestInvalid("ChangeBendCommand {}", "Missing model names");
    TestInvalid(R"(ChangeBendCommand { model_names: [ " BadName" ] })",
                "Invalid model name");
    TestInvalid(R"(ChangeBendCommand { model_names: ["X"], axis: 0 0 0 })",
                "Zero-length spin axis vector");
    TestValid(R"(ChangeBendCommand { model_names: ["Box"] })");
}

TEST_F(ChangeBendCommandTest, GetDescription) {
    auto cbc = ParseTypedObject<ChangeBendCommand>(
        R"(ChangeBendCommand { model_names: ["Box"] })");
    EXPECT_EQ(R"(Bent Model "Box")", cbc->GetDescription());

    cbc = ParseTypedObject<ChangeBendCommand>(
        R"(ChangeBendCommand { model_names: ["A", "B"] })");
    EXPECT_EQ(R"(Bent 2 Models)", cbc->GetDescription());
}
