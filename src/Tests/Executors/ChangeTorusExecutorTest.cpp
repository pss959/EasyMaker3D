//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeTorusCommand.h"
#include "Executors/ChangeTorusExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/TorusModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeTorusExecutorTest : public ExecutorTestBase {};

TEST_F(ChangeTorusExecutorTest, TypeName) {
    ChangeTorusExecutor exec;
    EXPECT_EQ("ChangeTorusCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeTorusExecutorTest, ChangeTorus) {
    auto icmd = ParseCommand<ChangeTorusCommand>(
        "ChangeTorusCommand",
        R"(model_names: ["Torus_0", "Torus_1"], new_radius: .4)");
    auto ocmd = ParseCommand<ChangeTorusCommand>(
        "ChangeTorusCommand",
        R"(model_names: ["Torus_0", "Torus_1"],
           new_radius: 4, is_inner_radius: false)");

    ChangeTorusExecutor exec;
    auto context = InitContext(exec);

    // Create and add two TorusModels.
    auto torus0 = Model::CreateModel<TorusModel>("Torus_0");
    auto torus1 = Model::CreateModel<TorusModel>("Torus_1");
    context.root_model->AddChildModel(torus0);
    context.root_model->AddChildModel(torus1);

    context.selection_manager->SelectAll();

    const auto old_inner0 = torus0->GetInnerRadius();
    const auto old_outer0 = torus0->GetOuterRadius();
    const auto old_inner1 = torus1->GetInnerRadius();
    const auto old_outer1 = torus1->GetOuterRadius();

    // Translate the models so that they are resting on the virtual "Stage"
    // (Y=0) to test that they are translated when the inner radius changes.
    const Vector3f trans0(0, old_inner0, 0);
    const Vector3f trans1(0, old_inner1, 0);
    torus0->SetTranslation(trans0);
    torus1->SetTranslation(trans1);

    EXPECT_EQ(old_inner0,                     torus0->GetInnerRadius());
    EXPECT_EQ(old_outer0,                     torus0->GetOuterRadius());
    EXPECT_EQ(old_inner1,                     torus1->GetInnerRadius());
    EXPECT_EQ(old_outer1,                     torus1->GetOuterRadius());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   torus0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, torus1->GetStatus());
    EXPECT_EQ(trans0,                         torus0->GetTranslation());
    EXPECT_EQ(trans1,                         torus1->GetTranslation());

    // Change the inner radii. Should also change translation.
    exec.Execute(*icmd, Command::Op::kDo);
    EXPECT_EQ(.4f,                            torus0->GetInnerRadius());
    EXPECT_EQ(old_outer0,                     torus0->GetOuterRadius());
    EXPECT_EQ(.4f,                            torus1->GetInnerRadius());
    EXPECT_EQ(old_outer1,                     torus1->GetOuterRadius());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   torus0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, torus1->GetStatus());
    EXPECT_EQ(Vector3f(0, .4f, 0),            torus0->GetTranslation());
    EXPECT_EQ(Vector3f(0, .4f, 0),            torus1->GetTranslation());

    // Change the outer radii.
    exec.Execute(*ocmd, Command::Op::kDo);
    EXPECT_EQ(.4f,                            torus0->GetInnerRadius());
    EXPECT_EQ(4,                              torus0->GetOuterRadius());
    EXPECT_EQ(.4f,                            torus1->GetInnerRadius());
    EXPECT_EQ(4,                              torus1->GetOuterRadius());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   torus0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, torus1->GetStatus());
    EXPECT_EQ(Vector3f(0, .4f, 0),            torus0->GetTranslation());
    EXPECT_EQ(Vector3f(0, .4f, 0),            torus1->GetTranslation());

    // Undo both (reverse order).
    exec.Execute(*ocmd, Command::Op::kUndo);
    EXPECT_EQ(.4f,                            torus0->GetInnerRadius());
    EXPECT_EQ(old_outer0,                     torus0->GetOuterRadius());
    EXPECT_EQ(.4f,                            torus1->GetInnerRadius());
    EXPECT_EQ(old_outer1,                     torus1->GetOuterRadius());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   torus0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, torus1->GetStatus());
    EXPECT_EQ(Vector3f(0, .4f, 0),            torus0->GetTranslation());
    EXPECT_EQ(Vector3f(0, .4f, 0),            torus1->GetTranslation());
    exec.Execute(*icmd, Command::Op::kUndo);
    EXPECT_EQ(old_inner0,                     torus0->GetInnerRadius());
    EXPECT_EQ(old_outer0,                     torus0->GetOuterRadius());
    EXPECT_EQ(old_inner1,                     torus1->GetInnerRadius());
    EXPECT_EQ(old_outer1,                     torus1->GetOuterRadius());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   torus0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, torus1->GetStatus());
    EXPECT_EQ(trans0,                         torus0->GetTranslation());
    EXPECT_EQ(trans1,                         torus1->GetTranslation());
}
