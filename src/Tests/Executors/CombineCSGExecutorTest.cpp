//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/CombineCSGModelCommand.h"
#include "Executors/CombineCSGExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/CSGModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class CombineCSGExecutorTest : public ExecutorTestBase {};

TEST_F(CombineCSGExecutorTest, TypeName) {
    CombineCSGExecutor exec;
    EXPECT_EQ("CombineCSGModelCommand", exec.GetCommandTypeName());
}

TEST_F(CombineCSGExecutorTest, CombineCSGWithoutName) {
    // Set up a CombineCSGModelCommand without a result name. This cannot be
    // created by parsing because having no result name is a parse error.
    auto cmd = Command::CreateCommand<CombineCSGModelCommand>();
    cmd->SetModelNames(StrVec{ "Box_0", "Box_1" });
    cmd->SetOperation(CSGOperation::kDifference);

    CombineCSGExecutor exec;
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
    auto csg = std::dynamic_pointer_cast<CSGModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(csg);
    EXPECT_EQ("Difference_1", csg->GetName());
    EXPECT_EQ(2U,             csg->GetChildModelCount());
    EXPECT_EQ(box0,           csg->GetChildModel(0));
    EXPECT_EQ(box1,           csg->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kPrimary,       csg->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kAncestorShown, box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kAncestorShown, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(2U,   context.root_model->GetChildModelCount());
    EXPECT_EQ(box0, context.root_model->GetChildModel(0));
    EXPECT_EQ(box1, context.root_model->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
}

TEST_F(CombineCSGExecutorTest, CombineCSGWithName) {
    auto cmd = ParseCommand<CombineCSGModelCommand>(
        "CombineCSGModelCommand",
        R"(model_names: ["Box_0", "Box_1"], operation: "kDifference",
           result_name: "Diff_0")");

    CombineCSGExecutor exec;
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
    auto csg = std::dynamic_pointer_cast<CSGModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(csg);
    EXPECT_EQ("Diff_0", csg->GetName());
    EXPECT_EQ(2U,       csg->GetChildModelCount());
    EXPECT_EQ(box0,     csg->GetChildModel(0));
    EXPECT_EQ(box1,     csg->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kPrimary,       csg->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kAncestorShown, box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kAncestorShown, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(2U,   context.root_model->GetChildModelCount());
    EXPECT_EQ(box0, context.root_model->GetChildModel(0));
    EXPECT_EQ(box1, context.root_model->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
}
