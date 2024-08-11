//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeOrderCommand.h"
#include "Executors/ChangeOrderExecutor.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeOrderExecutorTest : public ExecutorTestBase {};

TEST_F(ChangeOrderExecutorTest, TypeName) {
    ChangeOrderExecutor exec;
    EXPECT_EQ("ChangeOrderCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeOrderExecutorTest, ChangeOrder) {
    SetParseTypeName("ChangeOrderCommand");
    auto pcmd = ParseCommand<ChangeOrderCommand>(
        "ChangeOrderCommand", R"(model_name: "Box_0")");
    auto ncmd = ParseCommand<ChangeOrderCommand>(
        "ChangeOrderCommand", R"(model_name: "Box_0")");
    pcmd->SetIsPrevious(true);
    ncmd->SetIsPrevious(false);

    ChangeOrderExecutor exec;
    auto context = InitContext(exec);

    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    auto box1 = Model::CreateModel<BoxModel>("Box_1");
    context.root_model->AddChildModel(box0);
    context.root_model->AddChildModel(box1);

    EXPECT_EQ(2U,   context.root_model->GetChildModelCount());
    EXPECT_EQ(box0, context.root_model->GetChildModel(0));
    EXPECT_EQ(box1, context.root_model->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kUnselected, box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kUnselected, box1->GetStatus());

    // Move box0 after box1. box0 should now be selected.
    exec.Execute(*ncmd, Command::Op::kDo);
    EXPECT_EQ(2U,   context.root_model->GetChildModelCount());
    EXPECT_EQ(box1, context.root_model->GetChildModel(0));
    EXPECT_EQ(box0, context.root_model->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kPrimary,    box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kUnselected, box1->GetStatus());

    // Move box0 before box1.
    exec.Execute(*pcmd, Command::Op::kDo);
    EXPECT_EQ(2U,   context.root_model->GetChildModelCount());
    EXPECT_EQ(box0, context.root_model->GetChildModel(0));
    EXPECT_EQ(box1, context.root_model->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kPrimary,    box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kUnselected, box1->GetStatus());

    // Undo both.
    exec.Execute(*pcmd, Command::Op::kUndo);
    EXPECT_EQ(2U,   context.root_model->GetChildModelCount());
    EXPECT_EQ(box1, context.root_model->GetChildModel(0));
    EXPECT_EQ(box0, context.root_model->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kPrimary,    box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kUnselected, box1->GetStatus());
    exec.Execute(*ncmd, Command::Op::kUndo);
    EXPECT_EQ(2U,   context.root_model->GetChildModelCount());
    EXPECT_EQ(box0, context.root_model->GetChildModel(0));
    EXPECT_EQ(box1, context.root_model->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kPrimary,    box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kUnselected, box1->GetStatus());
}
