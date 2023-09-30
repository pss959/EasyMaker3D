#include "Commands/ChangeMirrorCommand.h"
#include "Executors/ChangeMirrorExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/MirroredModel.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeMirrorExecutorTest : public ExecutorTestBase {};

TEST_F(ChangeMirrorExecutorTest, TypeName) {
    ChangeMirrorExecutor exec;
    EXPECT_EQ("ChangeMirrorCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeMirrorExecutorTest, ChangeMirror) {
    // Command to move the plane +1 unit in X.
    auto cmd = ParseCommand<ChangeMirrorCommand>(
        "ChangeMirrorCommand",
        R"(model_names: ["Mirrored_0", "Mirrored_1", "Mirrored_2"],
           plane: 1 0 0 1)");

    ChangeMirrorExecutor exec;
    auto context = InitContext(exec);

    // Create 3 boxes:
    //  - Box_0 is selected first; everything is mirrored around its center.
    //  - Box_1 is translated by 10 in X before being converted to Mirrored_1;
    //      it should end up on the opposite side (-X).
    //  - Box_2 is translated by 10 in X after being converted to Mirrored_2;
    //      it should stay on the side it is on (+X).
    auto box0  = Model::CreateModel<BoxModel>("Box_0");
    auto box1  = Model::CreateModel<BoxModel>("Box_1");
    auto box2  = Model::CreateModel<BoxModel>("Box_2");
    auto mirrored0 = Model::CreateModel<MirroredModel>("Mirrored_0");
    auto mirrored1 = Model::CreateModel<MirroredModel>("Mirrored_1");
    auto mirrored2 = Model::CreateModel<MirroredModel>("Mirrored_2");
    box1->SetTranslation(Vector3f(10, 0, 0));
    mirrored0->SetOperandModel(box0);
    mirrored1->SetOperandModel(box1);
    mirrored2->SetOperandModel(box2);
    context.root_model->AddChildModel(mirrored0);
    context.root_model->AddChildModel(mirrored1);
    context.root_model->AddChildModel(mirrored2);
    mirrored1->SetTranslation(Vector3f(-10, 0, 0));  // Result of mirroring.
    mirrored2->SetTranslation(Vector3f( 10, 0, 0));

    context.selection_manager->SelectAll();

    const auto old_normal0 = mirrored0->GetPlaneNormal();
    const auto old_normal1 = mirrored1->GetPlaneNormal();
    const auto old_normal2 = mirrored2->GetPlaneNormal();

    EXPECT_ENUM_EQ(Model::Status::kPrimary,   mirrored0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, mirrored1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, mirrored2->GetStatus());
    EXPECT_EQ(Vector3f(  0, 0, 0),            mirrored0->GetTranslation());
    EXPECT_EQ(Vector3f(-10, 0, 0),            mirrored1->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 0, 0),            mirrored2->GetTranslation());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   mirrored0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, mirrored1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, mirrored2->GetStatus());
    EXPECT_EQ(Vector3f(1, 0, 0),              mirrored0->GetPlaneNormal());
    EXPECT_EQ(Vector3f(1, 0, 0),              mirrored1->GetPlaneNormal());
    EXPECT_EQ(Vector3f(1, 0, 0),              mirrored2->GetPlaneNormal());
    EXPECT_EQ(Vector3f( 2, 0, 0),             mirrored0->GetTranslation());
    EXPECT_EQ(Vector3f(-8, 0, 0),             mirrored1->GetTranslation());
    EXPECT_EQ(Vector3f(12, 0, 0),             mirrored2->GetTranslation());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   mirrored0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, mirrored1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, mirrored2->GetStatus());
    EXPECT_EQ(old_normal0,                    mirrored0->GetPlaneNormal());
    EXPECT_EQ(old_normal1,                    mirrored1->GetPlaneNormal());
    EXPECT_EQ(old_normal2,                    mirrored2->GetPlaneNormal());
    EXPECT_EQ(Vector3f(  0, 0, 0),            mirrored0->GetTranslation());
    EXPECT_EQ(Vector3f(-10, 0, 0),            mirrored1->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 0, 0),            mirrored2->GetTranslation());
}
