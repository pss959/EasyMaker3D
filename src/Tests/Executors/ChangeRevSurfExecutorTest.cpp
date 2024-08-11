//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeRevSurfCommand.h"
#include "Executors/ChangeRevSurfExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/RevSurfModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeRevSurfExecutorTest : public ExecutorTestBase {};

TEST_F(ChangeRevSurfExecutorTest, TypeName) {
    ChangeRevSurfExecutor exec;
    EXPECT_EQ("ChangeRevSurfCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeRevSurfExecutorTest, ChangeRevSurf) {
    auto cmd = ParseCommand<ChangeRevSurfCommand>(
        "ChangeRevSurfCommand",
        R"(model_names: ["Rev_0", "Rev_1"],
           profile_points: [.8 .6], sweep_angle: 270)");

    ChangeRevSurfExecutor exec;
    auto context = InitContext(exec);

    // Create and add two RevSurfModels.
    auto rev0 = Model::CreateModel<RevSurfModel>("Rev_0");
    auto rev1 = Model::CreateModel<RevSurfModel>("Rev_1");
    context.root_model->AddChildModel(rev0);
    context.root_model->AddChildModel(rev1);

    context.selection_manager->SelectAll();

    const auto old_prof0  = rev0->GetProfile();
    const auto old_prof1  = rev1->GetProfile();
    const auto old_angle0 = rev0->GetSweepAngle();
    const auto old_angle1 = rev1->GetSweepAngle();

    EXPECT_EQ(old_prof0,                      rev0->GetProfile());
    EXPECT_EQ(old_prof1,                      rev1->GetProfile());
    EXPECT_EQ(old_angle0,                     rev0->GetSweepAngle());
    EXPECT_EQ(old_angle1,                     rev1->GetSweepAngle());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   rev0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, rev1->GetStatus());

    const auto expected = RevSurfModel::CreateProfile(
        Profile::PointVec{ Point2f(.8f, .6f) });

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(expected,                       rev0->GetProfile());
    EXPECT_EQ(expected,                       rev1->GetProfile());
    EXPECT_EQ(Anglef::FromDegrees(270),       rev0->GetSweepAngle());
    EXPECT_EQ(Anglef::FromDegrees(270),       rev1->GetSweepAngle());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   rev0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, rev1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(old_prof0,                      rev0->GetProfile());
    EXPECT_EQ(old_prof1,                      rev1->GetProfile());
    EXPECT_EQ(old_angle0,                     rev0->GetSweepAngle());
    EXPECT_EQ(old_angle1,                     rev1->GetSweepAngle());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   rev0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, rev1->GetStatus());
}
