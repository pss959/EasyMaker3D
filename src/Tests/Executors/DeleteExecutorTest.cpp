#include "Commands/DeleteCommand.h"
#include "Executors/DeleteExecutor.h"
#include "Managers/ClipboardManager.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class DeleteExecutorTest : public ExecutorTestBase {};

TEST_F(DeleteExecutorTest, TypeName) {
    DeleteExecutor exec;
    EXPECT_EQ("DeleteCommand", exec.GetCommandTypeName());
}

TEST_F(DeleteExecutorTest, Delete) {
    auto cmd = ParseCommand<DeleteCommand>(
        "DeleteCommand", R"(model_names: ["Box_0", "Box_1"])");

    DeleteExecutor exec;
    auto context = InitContext(exec);

    // Create and add a couple of BoxModel instances.
    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    auto box1 = Model::CreateModel<BoxModel>("Box_1");
    context.root_model->AddChildModel(box0);
    context.root_model->AddChildModel(box1);

    context.selection_manager->SelectAll();

    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
    EXPECT_EQ(2U,   context.root_model->GetChildModelCount());
    EXPECT_EQ(box0, context.root_model->GetChildModel(0));
    EXPECT_EQ(box1, context.root_model->GetChildModel(1));
    EXPECT_TRUE(context.clipboard_manager->Get().empty());

    // Delete them - should not affect the ClipboardManager.
    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(0U, context.root_model->GetChildModelCount());
    EXPECT_TRUE(context.clipboard_manager->Get().empty());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
    EXPECT_EQ(2U,   context.root_model->GetChildModelCount());
    EXPECT_EQ(box0, context.root_model->GetChildModel(0));
    EXPECT_EQ(box1, context.root_model->GetChildModel(1));
    EXPECT_TRUE(context.clipboard_manager->Get().empty());
}
