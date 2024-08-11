//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeTaperCommand.h"
#include "Math/Taper.h"
#include "Models/TaperedModel.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeTaperCommandTest : public CommandTestBase {
  protected:
    ChangeTaperCommandTest() { SetParseTypeName("ChangeTaperCommand"); }
};

TEST_F(ChangeTaperCommandTest, Default) {
    auto ctc = Command::CreateCommand<ChangeTaperCommand>();
    EXPECT_EQ(Taper(), ctc->GetTaper());
}

TEST_F(ChangeTaperCommandTest, Set) {
    Taper taper;
    taper.axis    = Dim::kZ;
    taper.profile = Taper::CreateDefaultProfile();
    auto ctc = Command::CreateCommand<ChangeTaperCommand>();
    ctc->SetTaper(taper);
    EXPECT_EQ(taper, ctc->GetTaper());
}

TEST_F(ChangeTaperCommandTest, IsValid) {
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [" BadName"])", "Invalid model name");
    TestInvalid(R"(model_names: ["X"], profile_points: [2 1])",
                "Invalid profile");
    TestInvalid(R"(model_names: ["X"], profile_points: [0 1])",
                "Invalid profile");
    TestInvalid(R"(model_names: ["X"], profile_points: [0 1, .1 1])",
                "Invalid profile");
    TestValid(R"(model_names: ["Box"])");
    TestValid(R"(model_names: ["Box"], profile_points: [.1 1, .4 .5, .2 0])");
}

TEST_F(ChangeTaperCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Box"])",
             R"(Changed the taper of Model "Box")");
    TestDesc(R"(model_names: ["A", "B"])", "Changed the taper of 2 Models");
}
