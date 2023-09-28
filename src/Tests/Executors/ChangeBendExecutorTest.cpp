#include "Commands/ChangeBendCommand.h"
#include "Executors/ChangeBendExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/BentModel.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeBendExecutorTest : public ExecutorTestBase {};

TEST_F(ChangeBendExecutorTest, TypeName) {
    ChangeBendExecutor exec;
    EXPECT_EQ("ChangeBendCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeBendExecutorTest, ChangeBend) {
    auto cmd = ParseCommand<ChangeBendCommand>(
        "ChangeBendCommand",
        R"(model_names: ["Bent_0", "Bent_1"],
           center: 1 2 3, axis: 0 0 1, angle: 40, offset: -1)");

    ChangeBendExecutor exec;
    auto context = InitContext(exec);

    // Create and add a couple of BentModel instances created from BoxModel
    // instances.
    auto box0  = Model::CreateModel<BoxModel>("Box_0");
    auto box1  = Model::CreateModel<BoxModel>("Box_1");
    auto bent0 = Model::CreateModel<BentModel>("Bent_0");
    auto bent1 = Model::CreateModel<BentModel>("Bent_1");
    bent0->SetOperandModel(box0);
    bent1->SetOperandModel(box1);
    context.root_model->AddChildModel(bent0);
    context.root_model->AddChildModel(bent1);

    context.selection_manager->SelectAll();

    const auto old_spin0 = bent0->GetSpin();
    const auto old_spin1 = bent1->GetSpin();

    EXPECT_ENUM_EQ(Model::Status::kPrimary,   bent0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, bent1->GetStatus());

    Spin expected;
    expected.center = Point3f(1, 2, 3);
    expected.axis   = Vector3f(0, 0, 1);
    expected.angle  = Anglef::FromDegrees(40);
    expected.offset = -1;

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(expected, bent0->GetSpin());
    EXPECT_EQ(expected, bent1->GetSpin());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   bent0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, bent1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(old_spin0, bent0->GetSpin());
    EXPECT_EQ(old_spin1, bent1->GetSpin());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   bent0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, bent1->GetStatus());
}
