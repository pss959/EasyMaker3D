#include "Commands/ChangeTwistCommand.h"
#include "Executors/ChangeTwistExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/TwistedModel.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeTwistExecutorTest : public ExecutorTestBase {};

TEST_F(ChangeTwistExecutorTest, TypeName) {
    ChangeTwistExecutor exec;
    EXPECT_EQ("ChangeTwistCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeTwistExecutorTest, ChangeTwist) {
    auto cmd = ParseCommand<ChangeTwistCommand>(
        "ChangeTwistCommand",
        R"(model_names: ["Twisted_0", "Twisted_1"],
           center: 1 2 3, axis: 0 0 1, angle: 40, offset: -1)");

    ChangeTwistExecutor exec;
    auto context = InitContext(exec);

    // Create and add a couple of TwistedModel instances created from BoxModel
    // instances.
    auto box0  = Model::CreateModel<BoxModel>("Box_0");
    auto box1  = Model::CreateModel<BoxModel>("Box_1");
    auto twisted0 = Model::CreateModel<TwistedModel>("Twisted_0");
    auto twisted1 = Model::CreateModel<TwistedModel>("Twisted_1");
    twisted0->SetOperandModel(box0);
    twisted1->SetOperandModel(box1);
    context.root_model->AddChildModel(twisted0);
    context.root_model->AddChildModel(twisted1);

    context.selection_manager->SelectAll();

    const auto old_spin0 = twisted0->GetSpin();
    const auto old_spin1 = twisted1->GetSpin();

    EXPECT_ENUM_EQ(Model::Status::kPrimary,   twisted0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, twisted1->GetStatus());

    Spin expected;
    expected.center = Point3f(1, 2, 3);
    expected.axis   = Vector3f(0, 0, 1);
    expected.angle  = Anglef::FromDegrees(40);
    expected.offset = -1;

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(expected, twisted0->GetSpin());
    EXPECT_EQ(expected, twisted1->GetSpin());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   twisted0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, twisted1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(old_spin0, twisted0->GetSpin());
    EXPECT_EQ(old_spin1, twisted1->GetSpin());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   twisted0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, twisted1->GetStatus());
}
