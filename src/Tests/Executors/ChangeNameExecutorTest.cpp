//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeNameCommand.h"
#include "Executors/ChangeNameExecutor.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeNameExecutorTest : public ExecutorTestBase {};

TEST_F(ChangeNameExecutorTest, TypeName) {
    ChangeNameExecutor exec;
    EXPECT_EQ("ChangeNameCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeNameExecutorTest, ChangeName) {
    SetParseTypeName("ChangeNameCommand");
    auto cmd = ParseCommand<ChangeNameCommand>(
        "ChangeNameCommand", R"(model_name: "Box_0", new_name: "Buddy")");

    ChangeNameExecutor exec;
    auto context = InitContext(exec);

    auto box = Model::CreateModel<BoxModel>("Box_0");
    context.root_model->AddChildModel(box);
    context.name_manager->Add("Box_0");

    context.selection_manager->SelectAll();

    EXPECT_EQ("Box_0", box->GetName());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, box->GetStatus());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ("Buddy", box->GetName());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, box->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ("Box_0", box->GetName());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, box->GetStatus());
}
