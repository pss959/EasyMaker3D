#include "Commands/CreateRevSurfModelCommand.h"
#include "Executors/CreateRevSurfExecutor.h"
#include "Models/RevSurfModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class CreateRevSurfExecutorTest : public ExecutorTestBase {};

TEST_F(CreateRevSurfExecutorTest, TypeName) {
    CreateRevSurfExecutor exec;
    EXPECT_EQ("CreateRevSurfModelCommand", exec.GetCommandTypeName());
}

TEST_F(CreateRevSurfExecutorTest, CreateRevSurfWithoutName) {
    // Set up the CreateRevSurfModelCommand without a result name. This cannot
    // be created by parsing because having no result name is a parse error.
    auto cmd = Command::CreateCommand<CreateRevSurfModelCommand>();
    cmd->SetInitialScale(3);  // Will be overridden by CreateRevSurfExecutor.

    CreateRevSurfExecutor exec;
    auto context = InitContext(exec);

    EXPECT_EQ(0U, context.root_model->GetChildModelCount());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U, context.root_model->GetChildModelCount());
    auto rev = std::dynamic_pointer_cast<RevSurfModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(rev);
    EXPECT_EQ("RevSurf_1",                  rev->GetName());
    EXPECT_EQ(Vector3f(4, 4, 4),            rev->GetScale());
    EXPECT_EQ(Vector3f(0, 2, 0),            rev->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, rev->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(0U, context.root_model->GetChildModelCount());
}

TEST_F(CreateRevSurfExecutorTest, CreateRevSurfWithName) {
    auto cmd = ParseCommand<CreateRevSurfModelCommand>(
        "CreateRevSurfModelCommand",
        R"(result_name: "Rev_0", initial_scale: 3)");

    CreateRevSurfExecutor exec;
    auto context = InitContext(exec);

    EXPECT_EQ(0U, context.root_model->GetChildModelCount());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U, context.root_model->GetChildModelCount());
    auto rev = std::dynamic_pointer_cast<RevSurfModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(rev);
    EXPECT_EQ("Rev_0",                      rev->GetName());
    EXPECT_EQ(Vector3f(4, 4, 4),            rev->GetScale());
    EXPECT_EQ(Vector3f(0, 2, 0),            rev->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, rev->GetStatus());
}
