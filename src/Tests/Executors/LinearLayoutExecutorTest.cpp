//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/LinearLayoutCommand.h"
#include "Executors/LinearLayoutExecutor.h"
#include "Managers/ClipboardManager.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class LinearLayoutExecutorTest : public ExecutorTestBase {};

TEST_F(LinearLayoutExecutorTest, TypeName) {
    LinearLayoutExecutor exec;
    EXPECT_EQ("LinearLayoutCommand", exec.GetCommandTypeName());
}

TEST_F(LinearLayoutExecutorTest, LinearLayout) {
    auto cmd = ParseCommand<LinearLayoutCommand>(
        "LinearLayoutCommand",
        R"(model_names: ["Box_0", "Box_1", "Box_2"], offset: 10 0 0)");

    LinearLayoutExecutor exec;
    auto context = InitContext(exec);

    // Create and add 3 BoxModel instances.
    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    auto box1 = Model::CreateModel<BoxModel>("Box_1");
    auto box2 = Model::CreateModel<BoxModel>("Box_2");
    context.root_model->AddChildModel(box0);
    context.root_model->AddChildModel(box1);
    context.root_model->AddChildModel(box2);

    context.selection_manager->SelectAll();

    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box2->GetStatus());
    EXPECT_EQ(Vector3f(0, 0, 0), box0->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0), box1->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0), box2->GetTranslation());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box2->GetStatus());
    EXPECT_EQ(Vector3f( 0, 0, 0), box0->GetTranslation());
    EXPECT_EQ(Vector3f(10, 0, 0), box1->GetTranslation());
    EXPECT_EQ(Vector3f(20, 0, 0), box2->GetTranslation());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box2->GetStatus());
    EXPECT_EQ(Vector3f(0, 0, 0), box0->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0), box1->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0), box2->GetTranslation());
}
