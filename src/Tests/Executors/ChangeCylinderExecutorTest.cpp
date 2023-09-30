#include "Commands/ChangeCylinderCommand.h"
#include "Executors/ChangeCylinderExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/CylinderModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeCylinderExecutorTest : public ExecutorTestBase {};

TEST_F(ChangeCylinderExecutorTest, TypeName) {
    ChangeCylinderExecutor exec;
    EXPECT_EQ("ChangeCylinderCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeCylinderExecutorTest, ChangeCylinder) {
    auto tcmd = ParseCommand<ChangeCylinderCommand>(
        "ChangeCylinderCommand",
        R"(model_names: ["Cyl_0", "Cyl_1"], new_radius: 3)");
    auto bcmd = ParseCommand<ChangeCylinderCommand>(
        "ChangeCylinderCommand",
        R"(model_names: ["Cyl_0", "Cyl_1"],
           new_radius: 4, is_top_radius: false)");

    ChangeCylinderExecutor exec;
    auto context = InitContext(exec);

    // Create and add two CylinderModels.
    auto cyl0 = Model::CreateModel<CylinderModel>("Cyl_0");
    auto cyl1 = Model::CreateModel<CylinderModel>("Cyl_1");
    context.root_model->AddChildModel(cyl0);
    context.root_model->AddChildModel(cyl1);

    context.selection_manager->SelectAll();

    const auto old_top0 = cyl0->GetTopRadius();
    const auto old_bot0 = cyl0->GetBottomRadius();
    const auto old_top1 = cyl1->GetTopRadius();
    const auto old_bot1 = cyl1->GetBottomRadius();

    EXPECT_EQ(old_top0,                       cyl0->GetTopRadius());
    EXPECT_EQ(old_bot0,                       cyl0->GetBottomRadius());
    EXPECT_EQ(old_top1,                       cyl1->GetTopRadius());
    EXPECT_EQ(old_bot1,                       cyl1->GetBottomRadius());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   cyl0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, cyl1->GetStatus());

    // Change the top radii.
    exec.Execute(*tcmd, Command::Op::kDo);
    EXPECT_EQ(3,                              cyl0->GetTopRadius());
    EXPECT_EQ(old_bot0,                       cyl0->GetBottomRadius());
    EXPECT_EQ(3,                              cyl1->GetTopRadius());
    EXPECT_EQ(old_bot1,                       cyl1->GetBottomRadius());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   cyl0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, cyl1->GetStatus());

    // Change the bottom radii.
    exec.Execute(*bcmd, Command::Op::kDo);
    EXPECT_EQ(3,                              cyl0->GetTopRadius());
    EXPECT_EQ(4,                              cyl0->GetBottomRadius());
    EXPECT_EQ(3,                              cyl1->GetTopRadius());
    EXPECT_EQ(4,                             cyl1->GetBottomRadius());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   cyl0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, cyl1->GetStatus());

    // Undo both (reverse order).
    exec.Execute(*bcmd, Command::Op::kUndo);
    EXPECT_EQ(3,                              cyl0->GetTopRadius());
    EXPECT_EQ(old_bot0,                       cyl0->GetBottomRadius());
    EXPECT_EQ(3,                              cyl1->GetTopRadius());
    EXPECT_EQ(old_bot1,                       cyl1->GetBottomRadius());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   cyl0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, cyl1->GetStatus());
    exec.Execute(*tcmd, Command::Op::kUndo);
    EXPECT_EQ(old_top0,                       cyl0->GetTopRadius());
    EXPECT_EQ(old_bot0,                       cyl0->GetBottomRadius());
    EXPECT_EQ(old_top1,                       cyl1->GetTopRadius());
    EXPECT_EQ(old_bot1,                       cyl1->GetBottomRadius());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   cyl0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, cyl1->GetStatus());
}
