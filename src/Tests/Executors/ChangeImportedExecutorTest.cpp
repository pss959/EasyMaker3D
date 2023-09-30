#include "Commands/ChangeImportedModelCommand.h"
#include "Executors/ChangeImportedExecutor.h"
#include "Managers/SelectionManager.h"
#include "Models/ImportedModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ChangeImportedExecutorTest : public ExecutorTestBase {};

TEST_F(ChangeImportedExecutorTest, TypeName) {
    ChangeImportedExecutor exec;
    EXPECT_EQ("ChangeImportedModelCommand", exec.GetCommandTypeName());
}

TEST_F(ChangeImportedExecutorTest, ChangeImported) {
    // Use a valid STL file to test placement.
    const Str path = GetDataPath("box.stl").ToString();
    auto cmd = ParseCommand<ChangeImportedModelCommand>(
        "ChangeImportedModelCommand",
        R"(model_name: "Imp_0", new_path: ")" + path + "\"");

    ChangeImportedExecutor exec;
    auto context = InitContext(exec);

    // Create and add one ImportedModel.
    auto imp = Model::CreateModel<ImportedModel>("Imp_0");
    context.root_model->AddChildModel(imp);

    context.selection_manager->SelectAll();

    const auto old_path = imp->GetPath();

    EXPECT_EQ(old_path,                     imp->GetPath());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, imp->GetStatus());
    EXPECT_EQ(Vector3f(0, 0, 0),            imp->GetTranslation());

    // Translation should change to place the imported box.
    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(path,                         imp->GetPath());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, imp->GetStatus());
    EXPECT_EQ(Vector3f(0, 4, 0),            imp->GetTranslation());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(old_path,                     imp->GetPath());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, imp->GetStatus());
    EXPECT_EQ(Vector3f(0, 0, 0),            imp->GetTranslation());
}

TEST_F(ChangeImportedExecutorTest, ChangeImportedError) {
    // Use a nonsense STL file to test error handling.
    auto cmd = ParseCommand<ChangeImportedModelCommand>(
        "ChangeImportedModelCommand",
        R"(model_name: "Imp_0", new_path: "/a/b/c.stl")");

    ChangeImportedExecutor exec;
    auto context = InitContext(exec);

    // Create and add one ImportedModel.
    auto imp = Model::CreateModel<ImportedModel>("Imp_0");
    context.root_model->AddChildModel(imp);

    context.selection_manager->SelectAll();

    const auto old_path = imp->GetPath();

    EXPECT_EQ(old_path,                     imp->GetPath());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, imp->GetStatus());
    EXPECT_EQ(Vector3f(0, 0, 0),            imp->GetTranslation());

    // Translation should not change because of the error.
    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ("/a/b/c.stl",                 imp->GetPath());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, imp->GetStatus());
    EXPECT_EQ(Vector3f(0, 0, 0),            imp->GetTranslation());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(old_path,                     imp->GetPath());
    EXPECT_ENUM_EQ(Model::Status::kPrimary, imp->GetStatus());
    EXPECT_EQ(Vector3f(0, 0, 0),            imp->GetTranslation());
}
