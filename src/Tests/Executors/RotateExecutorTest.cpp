//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/RotateCommand.h"
#include "Executors/RotateExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class RotateExecutorTest : public ExecutorTestBase {};

TEST_F(RotateExecutorTest, TypeName) {
    RotateExecutor exec;
    EXPECT_EQ("RotateCommand", exec.GetCommandTypeName());
}

TEST_F(RotateExecutorTest, RotateInPlace) {
    auto cmd = ParseCommand<RotateCommand>(
        "RotateCommand",
        R"(model_names: ["Box_0", "Box_1"],
           rotation: 0 0 1 60, is_in_place: True)");

    RotateExecutor exec;
    auto context = InitContext(exec);

    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    auto box1 = Model::CreateModel<BoxModel>("Box_1");
    box0->SetTranslation(Vector3f(-10, 0, 0));
    box1->SetTranslation(Vector3f( 10, 0, 0));
    context.root_model->AddChildModel(box0);
    context.root_model->AddChildModel(box1);

    Rotationf rot = BuildRotation(0, 0, 1, 60);

    context.selection_manager->SelectAll();
    EXPECT_EQ(Rotationf::Identity(),          box0->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),          box1->GetRotation());
    EXPECT_EQ(Vector3f(-10, 0, 0),            box0->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 0, 0),            box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(rot,                            box0->GetRotation());
    EXPECT_EQ(rot,                            box1->GetRotation());
    EXPECT_EQ(Vector3f(-10, 0, 0),            box0->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 0, 0),            box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(Rotationf::Identity(),          box0->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),          box1->GetRotation());
    EXPECT_EQ(Vector3f(-10, 0, 0),            box0->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 0, 0),            box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
}

TEST_F(RotateExecutorTest, RotateNotInPlace) {
    auto cmd = ParseCommand<RotateCommand>(
        "RotateCommand",
        R"(model_names: ["Box_0", "Box_1"],
           rotation: 0 0 1 60, is_in_place: False)");

    RotateExecutor exec;
    auto context = InitContext(exec);

    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    auto box1 = Model::CreateModel<BoxModel>("Box_1");
    box0->SetTranslation(Vector3f(-10, 0, 0));
    box1->SetTranslation(Vector3f( 10, 0, 0));
    context.root_model->AddChildModel(box0);
    context.root_model->AddChildModel(box1);

    Rotationf rot = BuildRotation(0, 0, 1, 60);

    context.selection_manager->SelectAll();
    EXPECT_EQ(Rotationf::Identity(),            box0->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),            box1->GetRotation());
    EXPECT_EQ(Vector3f(-10, 0, 0),              box0->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 0, 0),              box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,     box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary,   box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(rot,                              box0->GetRotation());
    EXPECT_EQ(rot,                              box1->GetRotation());
    EXPECT_EQ(Vector3f(-10, 0, 0),              box0->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(0, 17.3205f, 0), box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,     box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary,   box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(Rotationf::Identity(),            box0->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),            box1->GetRotation());
    EXPECT_EQ(Vector3f(-10, 0, 0),              box0->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 0, 0),              box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,     box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary,   box1->GetStatus());
}
