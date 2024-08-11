//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/CombineHullModelCommand.h"
#include "Executors/CombineHullExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/HullModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class CombineHullExecutorTest : public ExecutorTestBase {};

TEST_F(CombineHullExecutorTest, TypeName) {
    CombineHullExecutor exec;
    EXPECT_EQ("CombineHullModelCommand", exec.GetCommandTypeName());
}

TEST_F(CombineHullExecutorTest, CombineHullWithoutName) {
    // Set up a CombineHullModelCommand without a result name. This cannot be
    // created by parsing because having no result name is a parse error.
    auto cmd = Command::CreateCommand<CombineHullModelCommand>();
    cmd->SetModelNames(StrVec{ "Box_0", "Box_1" });

    CombineHullExecutor exec;
    auto context = InitContext(exec);

    // Create and add a couple of BoxModel instances.
    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    auto box1 = Model::CreateModel<BoxModel>("Box_1");
    context.root_model->AddChildModel(box0);
    context.root_model->AddChildModel(box1);

    // Translate box1 into a reasonable position.
    box1->SetTranslation(Vector3f(6, 0, 0));

    context.selection_manager->SelectAll();

    EXPECT_EQ(2U,   context.root_model->GetChildModelCount());
    EXPECT_EQ(box0, context.root_model->GetChildModel(0));
    EXPECT_EQ(box1, context.root_model->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U, context.root_model->GetChildModelCount());
    auto hull = std::dynamic_pointer_cast<HullModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(hull);
    EXPECT_EQ("Hull_1", hull->GetName());
    EXPECT_EQ(2U,       hull->GetChildModelCount());
    EXPECT_EQ(box0,     hull->GetChildModel(0));
    EXPECT_EQ(box1,     hull->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kPrimary,       hull->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kAncestorShown, box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kAncestorShown, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(2U,   context.root_model->GetChildModelCount());
    EXPECT_EQ(box0, context.root_model->GetChildModel(0));
    EXPECT_EQ(box1, context.root_model->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
}

TEST_F(CombineHullExecutorTest, CombineHullWithName) {
    auto cmd = ParseCommand<CombineHullModelCommand>(
        "CombineHullModelCommand",
        R"(model_names: ["Box_0", "Box_1"], result_name: "Hull_0")");

    CombineHullExecutor exec;
    auto context = InitContext(exec);

    // Create and add a couple of BoxModel instances.
    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    auto box1 = Model::CreateModel<BoxModel>("Box_1");
    context.root_model->AddChildModel(box0);
    context.root_model->AddChildModel(box1);

    // Translate box1 into a reasonable position.
    box1->SetTranslation(Vector3f(1, 1, 1));

    context.selection_manager->SelectAll();

    EXPECT_EQ(2U,   context.root_model->GetChildModelCount());
    EXPECT_EQ(box0, context.root_model->GetChildModel(0));
    EXPECT_EQ(box1, context.root_model->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U, context.root_model->GetChildModelCount());
    auto hull = std::dynamic_pointer_cast<HullModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(hull);
    EXPECT_EQ("Hull_0", hull->GetName());
    EXPECT_EQ(2U,       hull->GetChildModelCount());
    EXPECT_EQ(box0,     hull->GetChildModel(0));
    EXPECT_EQ(box1,     hull->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kPrimary,       hull->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kAncestorShown, box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kAncestorShown, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(2U,   context.root_model->GetChildModelCount());
    EXPECT_EQ(box0, context.root_model->GetChildModel(0));
    EXPECT_EQ(box1, context.root_model->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
}
