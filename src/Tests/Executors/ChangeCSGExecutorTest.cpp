//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeCSGOperationCommand.h"
#include "Executors/ChangeCSGExecutor.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/CSGModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeCSGExecutorTest : public ExecutorTestBase {
  protected:
    CSGModelPtr csg0, csg1;

    /// Sets up and adds 2 CSGModel instances from 2 BoxModels each. csg0 is a
    /// union and csg1 is an intersection.
    void InitCSGModels(Executor::Context &context) {
        auto box0a  = Model::CreateModel<BoxModel>("Box_0A");
        auto box0b  = Model::CreateModel<BoxModel>("Box_0B");
        auto box1a  = Model::CreateModel<BoxModel>("Box_1A");
        auto box1b  = Model::CreateModel<BoxModel>("Box_1B");
        csg0 = Model::CreateModel<CSGModel>("Union_0");
        csg1 = Model::CreateModel<CSGModel>("Intersection_1");
        csg0->SetOperation(CSGOperation::kUnion);
        csg1->SetOperation(CSGOperation::kIntersection);
        csg0->SetOperandModels(std::vector<ModelPtr>{ box0a, box0b });
        csg1->SetOperandModels(std::vector<ModelPtr>{ box1a, box1b });
        context.root_model->AddChildModel(csg0);
        context.root_model->AddChildModel(csg1);
        context.name_manager->Add("Union_0");
        context.name_manager->Add("Intersection_1");
    }
};

TEST_F(ChangeCSGExecutorTest, TypeName) {
    ChangeCSGExecutor exec;
    EXPECT_EQ("ChangeCSGOperationCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeCSGExecutorTest, ChangeCSGWithoutNames) {
    // Set up a ChangeCSGOperationCommand without result names. This cannot be
    // created by parsing because having no result names is a parse error.
    auto cmd = Command::CreateCommand<ChangeCSGOperationCommand>();
    cmd->SetModelNames(StrVec{ "Union_0", "Intersection_1" });
    cmd->SetNewOperation(CSGOperation::kDifference);

    ChangeCSGExecutor exec;
    auto context = InitContext(exec);
    InitCSGModels(context);

    context.selection_manager->SelectAll();

    EXPECT_ENUM_EQ(Model::Status::kPrimary,   csg0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, csg1->GetStatus());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ("Difference_1",                 csg0->GetName());
    EXPECT_EQ("Difference_2",                 csg1->GetName());
    EXPECT_ENUM_EQ(CSGOperation::kDifference, csg0->GetOperation());
    EXPECT_ENUM_EQ(CSGOperation::kDifference, csg0->GetOperation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   csg0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, csg1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ("Union_0",                        csg0->GetName());
    EXPECT_EQ("Intersection_1",                 csg1->GetName());
    EXPECT_ENUM_EQ(CSGOperation::kUnion,        csg0->GetOperation());
    EXPECT_ENUM_EQ(CSGOperation::kIntersection, csg1->GetOperation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,     csg0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary,   csg1->GetStatus());
}

TEST_F(ChangeCSGExecutorTest, ChangeCSGWithNames) {
    auto cmd = ParseCommand<ChangeCSGOperationCommand>(
        "ChangeCSGOperationCommand",
        R"(model_names: ["Union_0", "Intersection_1"],
           result_names: ["Diff_0", "Diff_1"], new_operation: "kDifference")");

    ChangeCSGExecutor exec;
    auto context = InitContext(exec);
    InitCSGModels(context);

    context.selection_manager->SelectAll();

    EXPECT_ENUM_EQ(Model::Status::kPrimary,   csg0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, csg1->GetStatus());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ("Diff_0",                       csg0->GetName());
    EXPECT_EQ("Diff_1",                       csg1->GetName());
    EXPECT_ENUM_EQ(CSGOperation::kDifference, csg0->GetOperation());
    EXPECT_ENUM_EQ(CSGOperation::kDifference, csg0->GetOperation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   csg0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, csg1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ("Union_0",                        csg0->GetName());
    EXPECT_EQ("Intersection_1",                 csg1->GetName());
    EXPECT_ENUM_EQ(CSGOperation::kUnion,        csg0->GetOperation());
    EXPECT_ENUM_EQ(CSGOperation::kIntersection, csg1->GetOperation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,     csg0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary,   csg1->GetStatus());
}
