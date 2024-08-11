//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeImportedModelCommand.h"
#include "Tests/Commands/CommandTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeImportedModelCommandTest : public CommandTestBase {
  protected:
  protected:
    ChangeImportedModelCommandTest() {
        SetParseTypeName("ChangeImportedModelCommand");
    }
};

TEST_F(ChangeImportedModelCommandTest, Default) {
    auto cimc = Command::CreateCommand<ChangeImportedModelCommand>();
    EXPECT_TRUE(cimc->GetNewPath().empty());
}

TEST_F(ChangeImportedModelCommandTest, Set) {
    auto cimc = Command::CreateCommand<ChangeImportedModelCommand>();
    cimc->SetNewPath("/a/b/cd/e.stl");
    EXPECT_EQ("/a/b/cd/e.stl", cimc->GetNewPath());
}

TEST_F(ChangeImportedModelCommandTest, IsValid) {
    TestInvalid("",                          "Invalid model name");
    TestInvalid(R"(model_name: " BadName")", "Invalid model name");
    TestInvalid(R"(model_name: "Imp")",      "Missing new path to model");
    TestValid(R"(model_name: "Imp", new_path: "/a/b/c.stl")");
}

TEST_F(ChangeImportedModelCommandTest, GetDescription) {
    TestDesc(R"(model_name: "Imp", new_path: "/a/b/c.stl")",
             R"(Changed the import path in Model "Imp")");
}
