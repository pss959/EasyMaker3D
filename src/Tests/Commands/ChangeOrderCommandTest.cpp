//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeOrderCommand.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeOrderCommandTest : public CommandTestBase {
  protected:
    ChangeOrderCommandTest() { SetParseTypeName("ChangeOrderCommand"); }
};

TEST_F(ChangeOrderCommandTest, Default) {
    auto coc = Command::CreateCommand<ChangeOrderCommand>();
    EXPECT_FALSE(coc->IsPrevious());
}

TEST_F(ChangeOrderCommandTest, Set) {
    auto coc = Command::CreateCommand<ChangeOrderCommand>();
    coc->SetIsPrevious(true);
    EXPECT_TRUE(coc->IsPrevious());
}

TEST_F(ChangeOrderCommandTest, IsValid) {
    TestInvalid("",                          "Invalid model name");
    TestInvalid(R"(model_name: " BadName")", "Invalid model name");
    TestValid(R"(model_name: "Box")");
    TestValid(R"(model_name: "Box", is_previous: True)");
}

TEST_F(ChangeOrderCommandTest, GetDescription) {
    TestDesc(R"(model_name: "Box", is_previous: False)",
             R"(Moved Model "Box" later in the order)");
    TestDesc(R"(model_name: "Box", is_previous: True)",
             R"(Moved Model "Box" earlier in the order)");
}
