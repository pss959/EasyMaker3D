//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/CopyCommand.h"
#include "Executors/CopyExecutor.h"
#include "Managers/ClipboardManager.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class CopyExecutorTest : public ExecutorTestBase {};

TEST_F(CopyExecutorTest, TypeName) {
    CopyExecutor exec;
    EXPECT_EQ("CopyCommand", exec.GetCommandTypeName());
}

TEST_F(CopyExecutorTest, Copy) {
    auto cmd = ParseCommand<CopyCommand>(
        "CopyCommand", R"(model_names: ["Box_0", "Box_1"])");

    CopyExecutor exec;
    auto context = InitContext(exec);

    // Create and add a couple of BoxModel instances.
    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    auto box1 = Model::CreateModel<BoxModel>("Box_1");
    context.root_model->AddChildModel(box0);
    context.root_model->AddChildModel(box1);

    context.selection_manager->SelectAll();

    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
    EXPECT_TRUE(context.clipboard_manager->Get().empty());

    // Copy them - copies should end up in the ClipboardManager.
    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(2U,   context.root_model->GetChildModelCount());
    EXPECT_EQ(box0, context.root_model->GetChildModel(0));
    EXPECT_EQ(box1, context.root_model->GetChildModel(1));
    const auto &copies = context.clipboard_manager->Get();
    EXPECT_EQ(2U, copies.size());
    EXPECT_NE(box0,       copies[0]);
    EXPECT_NE(box1,       copies[1]);
    EXPECT_EQ("BoxModel", copies[0]->GetTypeName());
    EXPECT_EQ("BoxModel", copies[1]->GetTypeName());
    EXPECT_EQ("Box_0",    copies[0]->GetName());
    EXPECT_EQ("Box_1",    copies[1]->GetName());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());

    // Cannot undo a copy operation.
}
