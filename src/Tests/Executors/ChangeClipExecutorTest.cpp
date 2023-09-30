#include "Commands/ChangeClipCommand.h"
#include "Executors/ChangeClipExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/ClippedModel.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeClipExecutorTest : public ExecutorTestBase {};

TEST_F(ChangeClipExecutorTest, TypeName) {
    ChangeClipExecutor exec;
    EXPECT_EQ("ChangeClipCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeClipExecutorTest, ChangeClip) {
    auto cmd = ParseCommand<ChangeClipCommand>(
        "ChangeClipCommand",
        R"(model_names: ["Clipped_0", "Clipped_1"], plane: 1 0 0 .5)");

    ChangeClipExecutor exec;
    auto context = InitContext(exec);

    // Create and add a couple of ClippedModel instances created from BoxModel
    // instances.
    auto box0  = Model::CreateModel<BoxModel>("Box_0");
    auto box1  = Model::CreateModel<BoxModel>("Box_1");
    auto clipped0 = Model::CreateModel<ClippedModel>("Clipped_0");
    auto clipped1 = Model::CreateModel<ClippedModel>("Clipped_1");
    clipped0->SetOperandModel(box0);
    clipped1->SetOperandModel(box1);
    context.root_model->AddChildModel(clipped0);
    context.root_model->AddChildModel(clipped1);

    context.selection_manager->SelectAll();

    const auto old_plane0 = clipped0->GetPlane();
    const auto old_plane1 = clipped1->GetPlane();

    // XXXX Test changes in translation...

    EXPECT_ENUM_EQ(Model::Status::kPrimary,   clipped0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, clipped1->GetStatus());

    Plane expected(.5f, Vector3f(1, 0, 0));

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(expected, clipped0->GetPlane());
    EXPECT_EQ(expected, clipped1->GetPlane());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   clipped0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, clipped1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(old_plane0, clipped0->GetPlane());
    EXPECT_EQ(old_plane1, clipped1->GetPlane());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   clipped0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, clipped1->GetStatus());
}
