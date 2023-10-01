#include "Tests/Executors/ConvertExecutorTestBase.h"

#include "Commands/ConvertCommand.h"
#include "Executors/ConvertExecutorBase.h"
#include "Managers/SelectionManager.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ConvertExecutorTestBase.h"
#include "Tests/Testing.h"

void ConvertExecutorTestBase::Convert_(
    ConvertCommand &cmd, ConvertExecutorBase &exec,
    const Str &model_type_name, const Str &name_prefix, bool has_names) {

    // Optionally set up the ConvertCommand without result names. This cannot
    // be created by parsing because having no result names is a parse error.
    if (! has_names)
        cmd.SetModelNames(StrVec{ "Box_0", "Box_1" });

    auto context = InitContext(exec);

    // Create and add a couple of BoxModel instances.
    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    auto box1 = Model::CreateModel<BoxModel>("Box_1");
    context.root_model->AddChildModel(box0);
    context.root_model->AddChildModel(box1);

    context.selection_manager->SelectAll();

    EXPECT_EQ(2U,   context.root_model->GetChildModelCount());
    EXPECT_EQ(box0, context.root_model->GetChildModel(0));
    EXPECT_EQ(box1, context.root_model->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());

    exec.Execute(cmd, Command::Op::kDo);
    EXPECT_EQ(2U, context.root_model->GetChildModelCount());
    auto conv0 = std::dynamic_pointer_cast<ConvertedModel>(
        context.root_model->GetChildModel(0));
    auto conv1 = std::dynamic_pointer_cast<ConvertedModel>(
        context.root_model->GetChildModel(1));
    EXPECT_NOT_NULL(conv0);
    EXPECT_NOT_NULL(conv1);
    EXPECT_EQ(model_type_name,                    conv0->GetTypeName());
    EXPECT_EQ(model_type_name,                    conv1->GetTypeName());
    if (has_names) {
        EXPECT_EQ(name_prefix + "_0",             conv0->GetName());
        EXPECT_EQ(name_prefix + "_1",             conv1->GetName());
    }
    else {
        EXPECT_EQ(name_prefix + "_1",             conv0->GetName());
        EXPECT_EQ(name_prefix + "_2",             conv1->GetName());
    }
    EXPECT_EQ(box0,                               conv0->GetOperandModel());
    EXPECT_EQ(box1,                               conv1->GetOperandModel());
    EXPECT_ENUM_EQ(Model::Status::kPrimary,       conv0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary,     conv1->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kAncestorShown, box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kAncestorShown, box1->GetStatus());

    exec.Execute(cmd, Command::Op::kUndo);
    EXPECT_EQ(2U,   context.root_model->GetChildModelCount());
    EXPECT_EQ(box0, context.root_model->GetChildModel(0));
    EXPECT_EQ(box1, context.root_model->GetChildModel(1));
    EXPECT_ENUM_EQ(Model::Status::kPrimary,   box0->GetStatus());
    EXPECT_ENUM_EQ(Model::Status::kSecondary, box1->GetStatus());
}
