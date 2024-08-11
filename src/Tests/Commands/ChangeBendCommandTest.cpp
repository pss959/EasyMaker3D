//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeBendCommand.h"
#include "Math/Spin.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeBendCommandTest : public CommandTestBase {
  protected:
    ChangeBendCommandTest() { SetParseTypeName("ChangeBendCommand"); }
};

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
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [ " BadName" ])", "Invalid model name");
    TestInvalid(R"(model_names: ["X"], axis: 0 0 0)",
                "Zero-length spin axis vector");
    TestValid(R"(model_names: ["Box"])");
}

TEST_F(ChangeBendCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Box"])",    R"(Bent Model "Box")");
    TestDesc(R"(model_names: ["A", "B"])",   "Bent 2 Models");
}
