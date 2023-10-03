#include "Commands/RadialLayoutCommand.h"
#include "Executors/RadialLayoutExecutor.h"
#include "Managers/ClipboardManager.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class RadialLayoutExecutorTest : public ExecutorTestBase {};

TEST_F(RadialLayoutExecutorTest, TypeName) {
    RadialLayoutExecutor exec;
    EXPECT_EQ("RadialLayoutCommand", exec.GetCommandTypeName());
}


TEST_F(RadialLayoutExecutorTest, RadialLayoutOneModel) {
    auto cmd = ParseCommand<RadialLayoutCommand>(
        "RadialLayoutCommand",
        R"(model_names: ["Box_0"],
           center: 10 0 0, normal: 1 0 0, radius: 4, arc: 90 -180 )");

    RadialLayoutExecutor exec;
    auto context = InitContext(exec);

    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    context.root_model->AddChildModel(box0);

    context.selection_manager->SelectAll();

    EXPECT_ENUM_EQ(Model::Status::kPrimary, box0->GetStatus());
    EXPECT_EQ(Rotationf::Identity(),        box0->GetRotation());
    EXPECT_EQ(Vector3f(0, 0, 0),            box0->GetTranslation());

    const Rotationf rot =
        Rotationf::RotateInto(Vector3f::AxisY(), Vector3f::AxisX());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_ENUM_EQ(Model::Status::kPrimary, box0->GetStatus());
    EXPECT_EQ(rot,                          box0->GetRotation());
    EXPECT_VECS_CLOSE(Vector3f(11, 0, 0),   box0->GetTranslation());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_ENUM_EQ(Model::Status::kPrimary, box0->GetStatus());
    EXPECT_EQ(Rotationf::Identity(),        box0->GetRotation());
    EXPECT_EQ(Vector3f(0, 0, 0),            box0->GetTranslation());
}

TEST_F(RadialLayoutExecutorTest, RadialLayoutThreeModels) {
    auto cmd = ParseCommand<RadialLayoutCommand>(
        "RadialLayoutCommand",
        R"(model_names: ["Box_0", "Box_1", "Box_2"],
           center: 10 0 0, normal: 1 0 0, radius: 4, arc: 90 -180 )");

    RadialLayoutExecutor exec;
    auto context = InitContext(exec);

    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    auto box1 = Model::CreateModel<BoxModel>("Box_1");
    auto box2 = Model::CreateModel<BoxModel>("Box_2");
    context.root_model->AddChildModel(box0);
    context.root_model->AddChildModel(box1);
    context.root_model->AddChildModel(box2);

    context.selection_manager->SelectAll();

    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box2->GetStatus());
    EXPECT_EQ(Rotationf::Identity(),          box0->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),          box1->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),          box2->GetRotation());
    EXPECT_EQ(Vector3f(0, 0, 0),              box0->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0),              box1->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0),              box2->GetTranslation());

    const Rotationf rot =
        Rotationf::RotateInto(Vector3f::AxisY(), Vector3f::AxisX());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box2->GetStatus());
    EXPECT_EQ(rot,                            box0->GetRotation());
    EXPECT_EQ(rot,                            box1->GetRotation());
    EXPECT_EQ(rot,                            box2->GetRotation());
    EXPECT_VECS_CLOSE(Vector3f(11,  0, -4),   box0->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(11, -4,  0),   box1->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(11,  0,  4),   box2->GetTranslation());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box2->GetStatus());
    EXPECT_EQ(Rotationf::Identity(),          box0->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),          box1->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),          box2->GetRotation());
    EXPECT_EQ(Vector3f(0, 0, 0),              box0->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0),              box1->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0),              box2->GetTranslation());
}

TEST_F(RadialLayoutExecutorTest, RadialLayoutFullCircle) {
    auto cmd = ParseCommand<RadialLayoutCommand>(
        "RadialLayoutCommand",
        R"(model_names: ["Box_0", "Box_1", "Box_2", "Box_3"],
           center: 10 0 0, normal: 0 1 0, radius: 6, arc: 90 -360 )");

    RadialLayoutExecutor exec;
    auto context = InitContext(exec);

    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    auto box1 = Model::CreateModel<BoxModel>("Box_1");
    auto box2 = Model::CreateModel<BoxModel>("Box_2");
    auto box3 = Model::CreateModel<BoxModel>("Box_3");
    context.root_model->AddChildModel(box0);
    context.root_model->AddChildModel(box1);
    context.root_model->AddChildModel(box2);
    context.root_model->AddChildModel(box3);

    context.selection_manager->SelectAll();

    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box2->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box3->GetStatus());
    EXPECT_EQ(Rotationf::Identity(),          box0->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),          box1->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),          box2->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),          box3->GetRotation());
    EXPECT_EQ(Vector3f(0, 0, 0),              box0->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0),              box1->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0),              box2->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0),              box3->GetTranslation());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box2->GetStatus());
    EXPECT_EQ(Rotationf::Identity(),          box0->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),          box1->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),          box2->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),          box3->GetRotation());
    EXPECT_VECS_CLOSE(Vector3f(16, 1,  0),    box0->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(10, 1,  6),    box1->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f( 4, 1,  0),    box2->GetTranslation());
    EXPECT_VECS_CLOSE(Vector3f(10, 1, -6),    box3->GetTranslation());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box2->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box3->GetStatus());
    EXPECT_EQ(Rotationf::Identity(),          box0->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),          box1->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),          box2->GetRotation());
    EXPECT_EQ(Rotationf::Identity(),          box3->GetRotation());
    EXPECT_EQ(Vector3f(0, 0, 0),              box0->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0),              box1->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0),              box2->GetTranslation());
    EXPECT_EQ(Vector3f(0, 0, 0),              box3->GetTranslation());
}
