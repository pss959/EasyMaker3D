#include "Commands/CreatePrimitiveModelCommand.h"
#include "Executors/CreatePrimitiveExecutor.h"
#include "Managers/AnimationManager.h"
#include "Managers/TargetManager.h"
#include "Models/BoxModel.h"
#include "Models/CylinderModel.h"
#include "Models/RootModel.h"
#include "Models/TorusModel.h"
#include "Place/PointTarget.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"
#include "Util/Delay.h"
#include "Util/Tuning.h"

/// \ingroup Tests
class CreatePrimitiveExecutorTest : public ExecutorTestBase {};

TEST_F(CreatePrimitiveExecutorTest, TypeName) {
    CreatePrimitiveExecutor exec;
    EXPECT_EQ("CreatePrimitiveModelCommand", exec.GetCommandTypeName());
}

TEST_F(CreatePrimitiveExecutorTest, CreatePrimitiveWithoutName) {
    // Set up the CreatePrimitiveModelCommand without a result name. This cannot
    // be created by parsing because having no result name is a parse error.
    auto cmd = Command::CreateCommand<CreatePrimitiveModelCommand>();
    cmd->SetInitialScale(3);  // Will be overridden by CreatePrimitiveExecutor.
    cmd->SetType(PrimitiveType::kCylinder);

    CreatePrimitiveExecutor exec;
    auto context = InitContext(exec);

    EXPECT_EQ(0U, context.root_model->GetChildModelCount());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U, context.root_model->GetChildModelCount());
    auto cyl = std::dynamic_pointer_cast<CylinderModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(cyl);
    EXPECT_EQ("Cylinder_1",                 cyl->GetName());
    EXPECT_EQ(Vector3f(2, 2, 2),            cyl->GetScale());
    EXPECT_EQ(Vector3f(0, 2, 0),            cyl->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, cyl->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(0U, context.root_model->GetChildModelCount());
}

TEST_F(CreatePrimitiveExecutorTest, CreatePrimitiveWithName) {
    auto cmd = ParseCommand<CreatePrimitiveModelCommand>(
        "CreatePrimitiveModelCommand", R"(type: "kBox", result_name: "Box_0")");

    CreatePrimitiveExecutor exec;
    auto context = InitContext(exec);

    EXPECT_EQ(0U, context.root_model->GetChildModelCount());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U, context.root_model->GetChildModelCount());
    auto box = std::dynamic_pointer_cast<BoxModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(box);
    EXPECT_EQ("Box_0",                      box->GetName());
    EXPECT_EQ(Vector3f(2, 2, 2),            box->GetScale());
    EXPECT_EQ(Vector3f(0, 2, 0),            box->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, box->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(0U, context.root_model->GetChildModelCount());
}

TEST_F(CreatePrimitiveExecutorTest, CreatePrimitiveAtTarget) {
    auto cmd = ParseCommand<CreatePrimitiveModelCommand>(
        "CreatePrimitiveModelCommand",
        R"(type: "kSphere", result_name: "Sphere_0")");

    CreatePrimitiveExecutor exec;
    auto context = InitContext(exec, true);  // Need targets.

    // Set up a PointTarget in the TargetManager.
    PointTargetPtr pt = CreateObject<PointTarget>();
    pt->SetPosition(Point3f(4, 5, 6));
    pt->SetDirection(Vector3f(1, 0, 0));
    context.target_manager->SetPointTarget(*pt);
    context.target_manager->SetPointTargetVisible(true);

    EXPECT_EQ(0U, context.root_model->GetChildModelCount());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U, context.root_model->GetChildModelCount());
    auto sphere = context.root_model->GetChildModel(0);
    EXPECT_NOT_NULL(sphere);
    EXPECT_EQ("Sphere_0",                   sphere->GetName());
    EXPECT_EQ(Vector3f(2, 2, 2),            sphere->GetScale());
    EXPECT_EQ(Vector3f(6, 5, 6),            sphere->GetTranslation());
    EXPECT_EQ(Rotationf::RotateInto(Vector3f(0, 1, 0), Vector3f(1, 0, 0)),
              sphere->GetRotation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, sphere->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(0U, context.root_model->GetChildModelCount());
}

TEST_F(CreatePrimitiveExecutorTest, CreateTorus) {
    // Coverage for last primitive type.
    auto cmd = ParseCommand<CreatePrimitiveModelCommand>(
        "CreatePrimitiveModelCommand",
        R"(type: "kTorus", result_name: "Torus_0",
           target_position: 1 2 3, target_direction: 0 0 1)");

    // Mark the command as validating so that the target information in the
    // command is used.
    cmd->SetIsValidating(true);

    CreatePrimitiveExecutor exec;
    auto context = InitContext(exec);

    EXPECT_EQ(0U, context.root_model->GetChildModelCount());

    // Note that the initial Torus translation depends on the inner radius;
    // this translation is added to the target position.
    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U, context.root_model->GetChildModelCount());
    auto torus = std::dynamic_pointer_cast<TorusModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(torus);
    EXPECT_EQ("Torus_0",                    torus->GetName());
    EXPECT_EQ(Vector3f(2, 2, 2),            torus->GetScale());
    EXPECT_EQ(Vector3f(1, 2, 3.4f),         torus->GetTranslation());
    EXPECT_EQ(Rotationf::RotateInto(Vector3f(0, 1, 0), Vector3f(0, 0, 1)),
              torus->GetRotation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, torus->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(0U, context.root_model->GetChildModelCount());
}

TEST_F(CreatePrimitiveExecutorTest, Animate) {
    // Enable animation.
    Model::EnablePlacementAnimation(true);

    auto cmd = ParseCommand<CreatePrimitiveModelCommand>(
        "CreatePrimitiveModelCommand", R"(type: "kBox", result_name: "Box_0")");

    CreatePrimitiveExecutor exec;
    auto context = InitContext(exec);

    // Use a very short animation time to speed up the test.
    const float duration = .01f;  // Seconds.
    exec.SetAnimationDuration(duration);

    const float ah = TK::kModelAnimationHeight;

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U, context.root_model->GetChildModelCount());
    auto model = context.root_model->GetChildModel(0);
    EXPECT_NOT_NULL(model);

    // Model should start at the result translation + the starting animation
    // height and should be unselected until the animation completes.
    EXPECT_ENUM_EQ(Model::Status::kUnselected, model->GetStatus());
    EXPECT_EQ(Vector3f(0, ah + 2, 0),          model->GetTranslation());

    // Wait until the animation is about half-finished.
    Util::DelayThread(.5f * duration);
    context.animation_manager->ProcessUpdate();
    EXPECT_ENUM_EQ(Model::Status::kUnselected, model->GetStatus());
    const auto trans = model->GetTranslation();
    EXPECT_EQ(0,              trans[0]);
    // Tolerance here is extra large for coverage builds.
    EXPECT_NEAR(.5f * ah + 2, trans[1], .2f * ah);
    EXPECT_EQ(0,              trans[2]);

    // Wait until the animation is finished.
    Util::DelayThread(.5f * duration);
    context.animation_manager->ProcessUpdate();
    EXPECT_ENUM_EQ(Model::Status::kPrimary,  model->GetStatus());
    EXPECT_EQ(Vector3f(0, 2, 0),             model->GetTranslation());

    // Restore the default setting.
    Model::EnablePlacementAnimation(false);
}
