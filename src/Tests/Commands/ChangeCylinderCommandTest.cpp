//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeCylinderCommand.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeCylinderCommandTest : public CommandTestBase {
  protected:
    ChangeCylinderCommandTest() { SetParseTypeName("ChangeCylinderCommand"); }
};

TEST_F(ChangeCylinderCommandTest, Default) {
    auto ccc = Command::CreateCommand<ChangeCylinderCommand>();
    EXPECT_TRUE(ccc->IsTopRadius());
    EXPECT_EQ(0, ccc->GetNewRadius());
}

TEST_F(ChangeCylinderCommandTest, Set) {
    auto ccc = Command::CreateCommand<ChangeCylinderCommand>();
    ccc->SetIsTopRadius(false);
    ccc->SetNewRadius(3);
    EXPECT_FALSE(ccc->IsTopRadius());
    EXPECT_EQ(3, ccc->GetNewRadius());
}

TEST_F(ChangeCylinderCommandTest, IsValid) {
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [" BadName"])", "Invalid model name");
    TestValid(R"(model_names: ["Cyl"])");
}

TEST_F(ChangeCylinderCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Cyl"])",
             R"(Changed the top radius of Model "Cyl")");
    TestDesc(R"(model_names: ["Cyl0", "Cyl1"], is_top_radius: False)",
             "Changed the bottom radius of 2 Models");
}
