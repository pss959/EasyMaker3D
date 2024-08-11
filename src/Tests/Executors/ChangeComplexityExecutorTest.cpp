//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeComplexityCommand.h"
#include "Executors/ChangeComplexityExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/CylinderModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeComplexityExecutorTest : public ExecutorTestBase {};

TEST_F(ChangeComplexityExecutorTest, TypeName) {
    ChangeComplexityExecutor exec;
    EXPECT_EQ("ChangeComplexityCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeComplexityExecutorTest, ChangeComplexity) {
    auto cmd = ParseCommand<ChangeComplexityCommand>(
        "ChangeComplexityCommand",
        R"(model_names: ["Cyl_0", "Cyl_1", "Box_2"], new_complexity: .4)");

    ChangeComplexityExecutor exec;
    auto context = InitContext(exec);

    // Create and add two CylinderModels and one BoxModel. If one of the
    // CylinderModels is the primary selection, complexity can be changed.
    auto cyl0 = Model::CreateModel<CylinderModel>("Cyl_0");
    auto cyl1 = Model::CreateModel<CylinderModel>("Cyl_1");
    auto box2 = Model::CreateModel<BoxModel>("Box_2");
    context.root_model->AddChildModel(cyl0);
    context.root_model->AddChildModel(cyl1);
    context.root_model->AddChildModel(box2);

    context.selection_manager->SelectAll();

    const auto old_complexity0 = cyl0->GetComplexity();
    const auto old_complexity1 = cyl1->GetComplexity();
    const auto old_complexity2 = box2->GetComplexity();

    EXPECT_EQ(old_complexity0,                cyl0->GetComplexity());
    EXPECT_EQ(old_complexity1,                cyl1->GetComplexity());
    EXPECT_EQ(old_complexity2,                box2->GetComplexity());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   cyl0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, cyl1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box2->GetStatus());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(.4f,                            cyl0->GetComplexity());
    EXPECT_EQ(.4f,                            cyl1->GetComplexity());
    EXPECT_EQ(old_complexity2,                box2->GetComplexity());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   cyl0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, cyl1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box2->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(old_complexity0,                cyl0->GetComplexity());
    EXPECT_EQ(old_complexity1,                cyl1->GetComplexity());
    EXPECT_EQ(old_complexity2,                box2->GetComplexity());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   cyl0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, cyl1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box2->GetStatus());
}
