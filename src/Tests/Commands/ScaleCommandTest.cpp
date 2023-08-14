#include "Commands/ScaleCommand.h"
#include "Math/Types.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ScaleCommandTest : public CommandTestBase {};

TEST_F(ScaleCommandTest, Default) {
    auto sc = Command::CreateCommand<ScaleCommand>();
    EXPECT_EQ(Vector3f(1, 1, 1),                    sc->GetRatios());
    EXPECT_ENUM_EQ(ScaleCommand::Mode::kAsymmetric, sc->GetMode());
    EXPECT_TRUE(sc->GetModelNames().empty());
}

TEST_F(ScaleCommandTest, Set) {
    auto sc = Command::CreateCommand<ScaleCommand>();
    sc->SetRatios(Vector3f(2, 3, 4));
    sc->SetMode(ScaleCommand::Mode::kCenterSymmetric);
    EXPECT_EQ(Vector3f(2, 3, 4),                         sc->GetRatios());
    EXPECT_ENUM_EQ(ScaleCommand::Mode::kCenterSymmetric, sc->GetMode());
}

TEST_F(ScaleCommandTest, IsValid) {
    TestInvalid("ScaleCommand {}", "Missing model names");
    TestInvalid(R"(ScaleCommand { model_names: [ " BadName" ] })",
                "Invalid model name");
    TestInvalid(R"(ScaleCommand { model_names: [ "Box" ], ratios: 1 0 2 })",
                "Invalid scale by zero");
    TestInvalid(R"(ScaleCommand { model_names: [ "Box" ],
                    mode: "kCenterSymmetric", ratios: 1 1 -1 })",
                "Invalid negative symmetric scale");
    TestInvalid(R"(ScaleCommand { model_names: [ "Box" ],
                      mode: "kBaseSymmetric", ratios: 1 -1 1 })",
                "Invalid negative symmetric scale");
    TestValid(R"(ScaleCommand { model_names: ["Box"], ratios: -1 1 2 })");
}

TEST_F(ScaleCommandTest, GetDescription) {
    TestDesc(R"(ScaleCommand { model_names: ["Box"], ratios: 1 2 3 })",
             R"(Scaled Model "Box" asymmetrically)");
    TestDesc(
        R"(ScaleCommand { model_names: ["A", "B"], mode: "kBaseSymmetric" })",
        R"(Scaled 2 Models symmetrically about the base center)");
    TestDesc(
        R"(ScaleCommand { model_names: ["Hello"], mode: "kCenterSymmetric" })",
        R"(Scaled Model "Hello" symmetrically about the center)");
}
