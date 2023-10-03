#include "Commands/CreatePrimitiveModelCommand.h"
#include "Executors/CreatePrimitiveExecutor.h"
#include "Managers/TargetManager.h"
#include "Models/BoxModel.h"
#include "Models/CylinderModel.h"
#include "Models/RootModel.h"
#include "Models/TorusModel.h"
#include "Place/PointTarget.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

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
    EXPECT_EQ("Sphere_0",                    sphere->GetName());
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
        R"(type: "kTorus", result_name: "Torus_0")");

    CreatePrimitiveExecutor exec;
    auto context = InitContext(exec);

    EXPECT_EQ(0U, context.root_model->GetChildModelCount());

    // Note that the Torus translation depends on the inner radius.
    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U, context.root_model->GetChildModelCount());
    auto torus = std::dynamic_pointer_cast<TorusModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(torus);
    EXPECT_EQ("Torus_0",                    torus->GetName());
    EXPECT_EQ(Vector3f(2, 2, 2),            torus->GetScale());
    EXPECT_VECS_CLOSE(Vector3f(0, .4f, 0),  torus->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, torus->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(0U, context.root_model->GetChildModelCount());
}
