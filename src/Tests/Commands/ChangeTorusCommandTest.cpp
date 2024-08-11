//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeTorusCommand.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeTorusCommandTest : public CommandTestBase {
  protected:
    ChangeTorusCommandTest() { SetParseTypeName("ChangeTorusCommand"); }
};

TEST_F(ChangeTorusCommandTest, Default) {
    auto ctc = Command::CreateCommand<ChangeTorusCommand>();
    EXPECT_TRUE(ctc->IsInnerRadius());
    EXPECT_EQ(0, ctc->GetNewRadius());
}

TEST_F(ChangeTorusCommandTest, Set) {
    auto ctc = Command::CreateCommand<ChangeTorusCommand>();
    ctc->SetIsInnerRadius(false);
    ctc->SetNewRadius(3);
    EXPECT_FALSE(ctc->IsInnerRadius());
    EXPECT_EQ(3, ctc->GetNewRadius());
}

TEST_F(ChangeTorusCommandTest, IsValid) {
    TestInvalid("", "Missing model names");
    TestInvalid(R"(model_names: [" BadName"])", "Invalid model name");
    TestInvalid(R"(model_names: ["T"], new_radius: 0)",
                "Inner radius is smaller than minimum");
    TestInvalid(R"(model_names: ["T"], is_inner_radius: False, new_radius: .1)",
                "Outer radius is smaller than minimum");
    TestValid(R"(model_names: ["Torus"])");
}

TEST_F(ChangeTorusCommandTest, GetDescription) {
    TestDesc(R"(model_names: ["Torus"], new_radius: .2)",
             R"(Changed the inner radius of Model "Torus")");
    TestDesc(R"(model_names: ["Torus0", "Torus1"], is_inner_radius: False,
                new_radius: 4)",
             "Changed the outer radius of 2 Models");
}
