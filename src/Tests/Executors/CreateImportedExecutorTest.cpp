//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/CreateImportedModelCommand.h"
#include "Executors/CreateImportedExecutor.h"
#include "Models/ImportedModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class CreateImportedExecutorTest : public ExecutorTestBase {};

TEST_F(CreateImportedExecutorTest, TypeName) {
    CreateImportedExecutor exec;
    EXPECT_EQ("CreateImportedModelCommand", exec.GetCommandTypeName());
}

TEST_F(CreateImportedExecutorTest, CreateImportedWithoutName) {
    // Set up the CreateImportedModelCommand without a result name. This cannot
    // be created by parsing because having no result name is a parse error.
    auto cmd = Command::CreateCommand<CreateImportedModelCommand>();
    cmd->SetInitialScale(3);

    // Path has to be empty.

    CreateImportedExecutor exec;
    auto context = InitContext(exec);

    EXPECT_EQ(0U, context.root_model->GetChildModelCount());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U, context.root_model->GetChildModelCount());
    auto imp = std::dynamic_pointer_cast<ImportedModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(imp);
    EXPECT_EQ("Imported_1",                 imp->GetName());
    EXPECT_EQ("",                           imp->GetPath());
    EXPECT_EQ(Vector3f(3, 3, 3),            imp->GetScale());
    EXPECT_EQ(Vector3f(0, 6, 0),            imp->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, imp->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(0U, context.root_model->GetChildModelCount());
}

TEST_F(CreateImportedExecutorTest, CreateImportedWithName) {
    auto cmd = ParseCommand<CreateImportedModelCommand>(
        "CreateImportedModelCommand",
        R"(result_name: "Imp_0", initial_scale: 3, path: "/a/b/c.stl")");

    CreateImportedExecutor exec;
    auto context = InitContext(exec);

    EXPECT_EQ(0U, context.root_model->GetChildModelCount());

    // Mark the command as validating so that the path is set in the
    // ImportedModel.
    cmd->SetIsValidating(true);

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U, context.root_model->GetChildModelCount());
    auto imp = std::dynamic_pointer_cast<ImportedModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(imp);
    EXPECT_EQ("Imp_0",                      imp->GetName());
    EXPECT_EQ("/a/b/c.stl",                 imp->GetPath());
    EXPECT_EQ(Vector3f(3, 3, 3),            imp->GetScale());
    EXPECT_EQ(Vector3f(0, 6, 0),            imp->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, imp->GetStatus());
}
