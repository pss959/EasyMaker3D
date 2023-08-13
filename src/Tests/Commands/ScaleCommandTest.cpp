#include "Commands/ScaleCommand.h"
#include "Math/Types.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ScaleCommandTest : public SceneTestBase {};

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
    TEST_THROW(ParseObject<ScaleCommand>("ScaleCommand {}"),
               Parser::Exception, "Missing model names");
    TEST_THROW(ParseObject<ScaleCommand>(
                   R"(ScaleCommand { model_names: [ " BadName" ] })"),
               Parser::Exception, "Invalid model name");
    TEST_THROW(ParseObject<ScaleCommand>(
                   R"(ScaleCommand { model_names: [ "Box" ], ratios: 1 0 2 })"),
               Parser::Exception, "Invalid scale by zero");
    TEST_THROW(ParseObject<ScaleCommand>(
                   R"(ScaleCommand { model_names: [ "Box" ],
                          mode: "kCenterSymmetric", ratios: 1 1 -1 })"),
               Parser::Exception, "Invalid negative symmetric scale");
    TEST_THROW(ParseObject<ScaleCommand>(
                   R"(ScaleCommand { model_names: [ "Box" ],
                          mode: "kBaseSymmetric", ratios: 1 -1 1 })"),
               Parser::Exception, "Invalid negative symmetric scale");

    // This should not throw.
    auto sc = ParseObject<ScaleCommand>(
        R"(ScaleCommand { model_names: ["Box"], ratios: -1 1 2 })");
    EXPECT_NOT_NULL(sc);
}

TEST_F(ScaleCommandTest, GetDescription) {
    auto sc = ParseObject<ScaleCommand>(
        R"(ScaleCommand { model_names: ["Box"], ratios: 1 2 3 })");
    EXPECT_EQ(R"(Scaled Model "Box" asymmetrically)", sc->GetDescription());

    sc = ParseObject<ScaleCommand>(
        R"(ScaleCommand { model_names: ["A", "B"], mode: "kBaseSymmetric" })");
    EXPECT_EQ(R"(Scaled 2 Models symmetrically about the base center)",
              sc->GetDescription());

    sc = ParseObject<ScaleCommand>(
        R"(ScaleCommand { model_names: ["Hello"], mode: "kCenterSymmetric" })");
    EXPECT_EQ(R"(Scaled Model "Hello" symmetrically about the center)",
              sc->GetDescription());
}
