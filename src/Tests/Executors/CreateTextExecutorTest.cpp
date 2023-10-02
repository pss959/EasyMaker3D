#include "Commands/CreateTextModelCommand.h"
#include "Executors/CreateTextExecutor.h"
#include "Models/TextModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class CreateTextExecutorTest : public ExecutorTestBase {};

TEST_F(CreateTextExecutorTest, TypeName) {
    CreateTextExecutor exec;
    EXPECT_EQ("CreateTextModelCommand", exec.GetCommandTypeName());
}

TEST_F(CreateTextExecutorTest, CreateTextWithoutName) {
    // Set up the CreateTextModelCommand without a result name. This cannot
    // be created by parsing because having no result name is a parse error.
    auto cmd = Command::CreateCommand<CreateTextModelCommand>();
    cmd->SetInitialScale(3);  // Will be overridden by CreateTextExecutor.
    cmd->SetText("Hi");

    CreateTextExecutor exec;
    auto context = InitContext(exec);

    EXPECT_EQ(0U, context.root_model->GetChildModelCount());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U, context.root_model->GetChildModelCount());
    auto text = std::dynamic_pointer_cast<TextModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(text);
    EXPECT_EQ("Text_1",                     text->GetName());
    EXPECT_EQ("Hi",                         text->GetTextString());
    EXPECT_EQ(Vector3f(3, 3, 3),            text->GetScale());
    EXPECT_EQ(Vector3f(0, 3, 0),            text->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, text->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(0U, context.root_model->GetChildModelCount());
}

TEST_F(CreateTextExecutorTest, CreateTextWithName) {
    auto cmd = ParseCommand<CreateTextModelCommand>(
        "CreateTextModelCommand",
        R"(result_name: "Text_0", initial_scale: 3, text: "Hi")");

    CreateTextExecutor exec;
    auto context = InitContext(exec);

    EXPECT_EQ(0U, context.root_model->GetChildModelCount());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U, context.root_model->GetChildModelCount());
    auto text = std::dynamic_pointer_cast<TextModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(text);
    EXPECT_EQ("Text_0",                     text->GetName());
    EXPECT_EQ("Hi",                         text->GetTextString());
    EXPECT_EQ(Vector3f(3, 3, 3),            text->GetScale());
    EXPECT_EQ(Vector3f(0, 3, 0),            text->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, text->GetStatus());
}
