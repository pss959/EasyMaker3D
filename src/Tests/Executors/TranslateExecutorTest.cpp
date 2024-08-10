#include "Commands/TranslateCommand.h"
#include "Executors/TranslateExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class TranslateExecutorTest : public ExecutorTestBase {};

TEST_F(TranslateExecutorTest, TypeName) {
    TranslateExecutor exec;
    EXPECT_EQ("TranslateCommand", exec.GetCommandTypeName());
}

TEST_F(TranslateExecutorTest, Translate) {
    auto cmd = ParseCommand<TranslateCommand>(
        "TranslateCommand",
        R"(model_names: ["Box_0", "Box_1"], translation: 3 4 5)");

    TranslateExecutor exec;
    auto context = InitContext(exec);

    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    auto box1 = Model::CreateModel<BoxModel>("Box_1");
    box0->SetTranslation(Vector3f(-10, 0, 0));
    box1->SetTranslation(Vector3f( 10, 0, 0));
    context.root_model->AddChildModel(box0);
    context.root_model->AddChildModel(box1);

    context.selection_manager->SelectAll();
    EXPECT_EQ(Vector3f(-10, 0, 0),            box0->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 0, 0),            box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(Vector3f(-7, 4, 5),             box0->GetTranslation());
    EXPECT_EQ(Vector3f(13, 4, 5),             box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(Vector3f(-10, 0, 0),            box0->GetTranslation());
    EXPECT_EQ(Vector3f( 10, 0, 0),            box1->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
}
