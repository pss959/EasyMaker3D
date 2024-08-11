//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeColorCommand.h"
#include "Executors/ChangeColorExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeColorExecutorTest : public ExecutorTestBase {};

TEST_F(ChangeColorExecutorTest, TypeName) {
    ChangeColorExecutor exec;
    EXPECT_EQ("ChangeColorCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeColorExecutorTest, ChangeColor) {
    auto cmd = ParseCommand<ChangeColorCommand>(
        "ChangeColorCommand",
        R"(model_names: ["Box_0", "Box_1"], new_color: 1 1 0 1)");

    ChangeColorExecutor exec;
    auto context = InitContext(exec);

    // Create and add a couple of BoxModel instances.
    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    auto box1 = Model::CreateModel<BoxModel>("Box_1");
    context.root_model->AddChildModel(box0);
    context.root_model->AddChildModel(box1);

    context.selection_manager->SelectAll();

    const auto old_color0 = box0->GetColor();
    const auto old_color1 = box1->GetColor();

    EXPECT_EQ(old_color0, box0->GetColor());
    EXPECT_EQ(old_color1, box1->GetColor());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(Color(1, 1, 0, 1), box0->GetColor());
    EXPECT_EQ(Color(1, 1, 0, 1), box1->GetColor());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(old_color0, box0->GetColor());
    EXPECT_EQ(old_color1, box1->GetColor());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
}
