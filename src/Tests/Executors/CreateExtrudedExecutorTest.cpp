//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/CreateExtrudedModelCommand.h"
#include "Executors/CreateExtrudedExecutor.h"
#include "Models/ExtrudedModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class CreateExtrudedExecutorTest : public ExecutorTestBase {};

TEST_F(CreateExtrudedExecutorTest, TypeName) {
    CreateExtrudedExecutor exec;
    EXPECT_EQ("CreateExtrudedModelCommand", exec.GetCommandTypeName());
}

TEST_F(CreateExtrudedExecutorTest, CreateExtrudedWithoutName) {
    // Set up the CreateExtrudedModelCommand without a result name. This cannot
    // be created by parsing because having no result name is a parse error.
    auto cmd = Command::CreateCommand<CreateExtrudedModelCommand>();
    cmd->SetInitialScale(3);

    CreateExtrudedExecutor exec;
    auto context = InitContext(exec);

    EXPECT_EQ(0U, context.root_model->GetChildModelCount());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U, context.root_model->GetChildModelCount());
    auto ext = std::dynamic_pointer_cast<ExtrudedModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(ext);
    EXPECT_EQ("Extruded_1",                 ext->GetName());
    EXPECT_EQ(Vector3f(3, 3, 3),            ext->GetScale());
    EXPECT_EQ(Vector3f(0, 3, 0),            ext->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, ext->GetStatus());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(0U, context.root_model->GetChildModelCount());
}

TEST_F(CreateExtrudedExecutorTest, CreateExtrudedWithName) {
    auto cmd = ParseCommand<CreateExtrudedModelCommand>(
        "CreateExtrudedModelCommand",
        R"(result_name: "Ext_0", initial_scale: 3)");

    CreateExtrudedExecutor exec;
    auto context = InitContext(exec);

    EXPECT_EQ(0U, context.root_model->GetChildModelCount());

    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U, context.root_model->GetChildModelCount());
    auto ext = std::dynamic_pointer_cast<ExtrudedModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(ext);
    EXPECT_EQ("Ext_0",                      ext->GetName());
    EXPECT_EQ(Vector3f(3, 3, 3),            ext->GetScale());
    EXPECT_EQ(Vector3f(0, 3, 0),            ext->GetTranslation());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, ext->GetStatus());
}
