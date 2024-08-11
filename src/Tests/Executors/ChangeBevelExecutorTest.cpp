//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeBevelCommand.h"
#include "Executors/ChangeBevelExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/BeveledModel.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeBevelExecutorTest : public ExecutorTestBase {};

TEST_F(ChangeBevelExecutorTest, TypeName) {
    ChangeBevelExecutor exec;
    EXPECT_EQ("ChangeBevelCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeBevelExecutorTest, ChangeBevel) {
    auto cmd = ParseCommand<ChangeBevelCommand>(
        "ChangeBevelCommand",
        R"(model_names: ["Beveled_0", "Beveled_1"],
           profile_points: [.4 .2], bevel_scale: 2, max_angle: 100)");

    ChangeBevelExecutor exec;
    auto context = InitContext(exec);

    // Create and add a couple of BeveledModel instances created from BoxModel
    // instances.
    auto box0  = Model::CreateModel<BoxModel>("Box_0");
    auto box1  = Model::CreateModel<BoxModel>("Box_1");
    auto beveled0 = Model::CreateModel<BeveledModel>("Beveled_0");
    auto beveled1 = Model::CreateModel<BeveledModel>("Beveled_1");
    beveled0->SetOperandModel(box0);
    beveled1->SetOperandModel(box1);
    context.root_model->AddChildModel(beveled0);
    context.root_model->AddChildModel(beveled1);

    context.selection_manager->SelectAll();

    const auto old_bevel0 = beveled0->GetBevel();
    const auto old_bevel1 = beveled1->GetBevel();

    EXPECT_ENUM_EQ(Model::Status::kPrimary,   beveled0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, beveled1->GetStatus());

    Bevel expected;

    Profile::PointVec pts{ Point2f(.4f, .2f) };
    expected.profile   = BeveledModel::CreateProfile(pts);
    expected.scale     = 2;
    expected.max_angle = Anglef::FromDegrees(100);

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(expected, beveled0->GetBevel());
    EXPECT_EQ(expected, beveled1->GetBevel());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   beveled0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, beveled1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(old_bevel0, beveled0->GetBevel());
    EXPECT_EQ(old_bevel1, beveled1->GetBevel());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   beveled0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, beveled1->GetStatus());
}
