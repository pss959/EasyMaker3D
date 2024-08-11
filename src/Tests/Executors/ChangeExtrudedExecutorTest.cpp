//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeExtrudedCommand.h"
#include "Executors/ChangeExtrudedExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/ExtrudedModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeExtrudedExecutorTest : public ExecutorTestBase {};

TEST_F(ChangeExtrudedExecutorTest, TypeName) {
    ChangeExtrudedExecutor exec;
    EXPECT_EQ("ChangeExtrudedCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeExtrudedExecutorTest, ChangeExtruded) {
    auto cmd = ParseCommand<ChangeExtrudedCommand>(
        "ChangeExtrudedCommand",
        R"(model_names: ["Ext_0", "Ext_1"],
           profile_points: [.9 .5, .3 .8, .3 .2])");

    ChangeExtrudedExecutor exec;
    auto context = InitContext(exec);

    // Create and add two ExtrudedModels.
    auto ext0 = Model::CreateModel<ExtrudedModel>("Ext_0");
    auto ext1 = Model::CreateModel<ExtrudedModel>("Ext_1");
    context.root_model->AddChildModel(ext0);
    context.root_model->AddChildModel(ext1);

    context.selection_manager->SelectAll();

    const auto old_prof0 = ext0->GetProfile();
    const auto old_prof1 = ext1->GetProfile();

    Profile::PointVec pts{
        Point2f(.9f, .5f), Point2f(.3f, .8f), Point2f(.3f, .2f) };
    Profile expected = ExtrudedModel::CreateProfile(pts);

    EXPECT_EQ(old_prof0,                      ext0->GetProfile());
    EXPECT_EQ(old_prof1,                      ext1->GetProfile());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   ext0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, ext1->GetStatus());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(expected,                       ext0->GetProfile());
    EXPECT_EQ(expected,                       ext1->GetProfile());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   ext0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, ext1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(old_prof0,                      ext0->GetProfile());
    EXPECT_EQ(old_prof1,                      ext1->GetProfile());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   ext0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, ext1->GetStatus());
}
