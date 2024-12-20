//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ScaleCommand.h"
#include "Math/Types.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ScaleCommandTest : public CommandTestBase {
  protected:
    ScaleCommandTest() { SetParseTypeName("ScaleCommand"); }
};

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
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [ " BadName" ])", "Invalid model name");
    TestInvalid(R"(model_names: [ "Box" ], ratios: 1 0 2)",
                "Invalid scale by zero");
    TestInvalid(R"(model_names: [ "Box" ],
                   mode: "kCenterSymmetric", ratios: 1 1 -1)",
                "Invalid negative symmetric scale");
    TestInvalid(R"(model_names: [ "Box" ],
                   mode: "kBaseSymmetric", ratios: 1 -1 1)",
                "Invalid negative symmetric scale");
    TestValid(R"(model_names: ["Box"], ratios: -1 1 2)");
}

TEST_F(ScaleCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Box"], ratios: 1 2 3)",
             R"(Scaled Model "Box" asymmetrically)");
    TestDesc(R"(model_names: ["A", "B"], mode: "kBaseSymmetric")",
             R"(Scaled 2 Models symmetrically about the base center)");
    TestDesc(R"(model_names: ["Hello"], mode: "kCenterSymmetric")",
             R"(Scaled Model "Hello" symmetrically about the center)");
}
