//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeColorCommand.h"
#include "Math/Types.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeColorCommandTest : public CommandTestBase {
  protected:
    ChangeColorCommandTest() { SetParseTypeName("ChangeColorCommand"); }
};

TEST_F(ChangeColorCommandTest, Default) {
    auto ccc = Command::CreateCommand<ChangeColorCommand>();
    EXPECT_EQ(Color(), ccc->GetNewColor());
}

TEST_F(ChangeColorCommandTest, Set) {
    auto ccc = Command::CreateCommand<ChangeColorCommand>();
    ccc->SetNewColor(Color(1, 1, .5f));
    EXPECT_EQ(Color(1, 1, .5f), ccc->GetNewColor());
}

TEST_F(ChangeColorCommandTest, IsValid) {
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [" BadName"])", "Invalid model name");
    TestValid(R"(model_names: ["Box"])");
}

TEST_F(ChangeColorCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Box"])", R"(Changed the color of Model "Box")");
    TestDesc(R"(model_names: ["A", "B"])", "Changed the color of 2 Models");
}
