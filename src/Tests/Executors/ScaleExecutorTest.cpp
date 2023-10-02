#include "Commands/ScaleCommand.h"
#include "Executors/ScaleExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ScaleExecutorTest : public ExecutorTestBase {
  protected:
    BoxModelPtr box0, box1;

    /// Creates and adds a couple of BoxModel instances with different scales.
    /// Translates box0 to the left by 10 and box1 to the right by 10 and in Y
    /// to rest on Y=0. Selects both.
    void InitBoxes(ScaleExecutor &exec);
};

void ScaleExecutorTest::InitBoxes(ScaleExecutor &exec) {
    auto context = InitContext(exec);

    box0 = Model::CreateModel<BoxModel>("Box_0");
    box1 = Model::CreateModel<BoxModel>("Box_1");
    box0->SetScale(Vector3f(1, 2, 3));
    box1->SetScale(Vector3f(2, 3, 4));
    box0->SetTranslation(Vector3f(-10, 2, 0));
    box1->SetTranslation(Vector3f( 10, 3, 0));
    context.root_model->AddChildModel(box0);
    context.root_model->AddChildModel(box1);

    context.selection_manager->SelectAll();
}

TEST_F(ScaleExecutorTest, TypeName) {
    ScaleExecutor exec;
    EXPECT_EQ("ScaleCommand", exec.GetCommandTypeName());
}

TEST_F(ScaleExecutorTest, ScaleAsymmetric) {
    auto cmd = ParseCommand<ScaleCommand>(
        "ScaleCommand",
        R"(model_names: ["Box_0", "Box_1"],
           mode: "kAsymmetric", ratios: 3 4 5)");

    ScaleExecutor exec;
    InitBoxes(exec);

    EXPECT_EQ(Vector3f(1, 2, 3),              box0->GetScale());
    EXPECT_EQ(Vector3f(2, 3, 4),              box1->GetScale());
    EXPECT_EQ(Vector3f(-10, 2, 0),            box0->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 3, 0),            box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(Vector3f(3,   8, 15),           box0->GetScale());
    EXPECT_EQ(Vector3f(6,  12, 20),           box1->GetScale());
    EXPECT_EQ(Vector3f(-8,  8, 12),           box0->GetTranslation());
    EXPECT_EQ(Vector3f(14, 12, 16),           box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(Vector3f(1, 2, 3),              box0->GetScale());
    EXPECT_EQ(Vector3f(2, 3, 4),              box1->GetScale());
    EXPECT_EQ(Vector3f(-10, 2, 0),            box0->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 3, 0),            box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
}

TEST_F(ScaleExecutorTest, ScaleCenterSymmetric) {
    auto cmd = ParseCommand<ScaleCommand>(
        "ScaleCommand",
        R"(model_names: ["Box_0", "Box_1"],
           mode: "kCenterSymmetric", ratios: 3 4 5)");

    ScaleExecutor exec;
    InitBoxes(exec);

    EXPECT_EQ(Vector3f(1, 2, 3),              box0->GetScale());
    EXPECT_EQ(Vector3f(2, 3, 4),              box1->GetScale());
    EXPECT_EQ(Vector3f(-10, 2, 0),            box0->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 3, 0),            box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(Vector3f(3,  8, 15),            box0->GetScale());
    EXPECT_EQ(Vector3f(6, 12, 20),            box1->GetScale());
    EXPECT_EQ(Vector3f(-10, 2, 0),            box0->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 3, 0),            box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(Vector3f(1, 2, 3),              box0->GetScale());
    EXPECT_EQ(Vector3f(2, 3, 4),              box1->GetScale());
    EXPECT_EQ(Vector3f(-10, 2, 0),            box0->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 3, 0),            box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
}

TEST_F(ScaleExecutorTest, ScaleBaseSymmetric) {
    auto cmd = ParseCommand<ScaleCommand>(
        "ScaleCommand",
        R"(model_names: ["Box_0", "Box_1"],
           mode: "kBaseSymmetric", ratios: 3 4 5)");

    ScaleExecutor exec;
    InitBoxes(exec);

    EXPECT_EQ(Vector3f(1, 2, 3),              box0->GetScale());
    EXPECT_EQ(Vector3f(2, 3, 4),              box1->GetScale());
    EXPECT_EQ(Vector3f(-10, 2, 0),            box0->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 3, 0),            box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(Vector3f(3,  8, 15),            box0->GetScale());
    EXPECT_EQ(Vector3f(6, 12, 20),            box1->GetScale());
    EXPECT_EQ(Vector3f(-10,  8, 0),           box0->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 12, 0),           box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(Vector3f(1, 2, 3),              box0->GetScale());
    EXPECT_EQ(Vector3f(2, 3, 4),              box1->GetScale());
    EXPECT_EQ(Vector3f(-10, 2, 0),            box0->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 3, 0),            box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
}
