#include "Commands/PasteCommand.h"
#include "Executors/PasteExecutor.h"
#include "Managers/ClipboardManager.h"
#include "Models/BoxModel.h"
#include "Models/HullModel.h"
#include "Models/RootModel.h"
#include "Tests/Executors/ExecutorTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class PasteExecutorTest : public ExecutorTestBase {};

TEST_F(PasteExecutorTest, TypeName) {
    PasteExecutor exec;
    EXPECT_EQ("PasteCommand", exec.GetCommandTypeName());
}

TEST_F(PasteExecutorTest, Paste) {
    auto cmd = ParseCommand<PasteCommand>("PasteCommand", "");

    PasteExecutor exec;
    auto context = InitContext(exec);

    // Create a couple of BoxModel instances and store copies in the
    // ClipboardManager.
    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    auto box1 = Model::CreateModel<BoxModel>("Box_1");
    context.clipboard_manager->StoreCopies(std::vector<ModelPtr>{ box0, box1 });

    EXPECT_EQ(0U, context.root_model->GetChildModelCount());
    EXPECT_EQ(2U, context.clipboard_manager->Get().size());

    // Paste them as top-level Models. Should not affect the ClipboardManager.
    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(2U,        context.root_model->GetChildModelCount());
    EXPECT_EQ("Box_0_A", context.root_model->GetChildModel(0)->GetName());
    EXPECT_EQ("Box_1_A", context.root_model->GetChildModel(1)->GetName());
    EXPECT_EQ(2U,        context.clipboard_manager->Get().size());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(0U, context.root_model->GetChildModelCount());
    EXPECT_EQ(2U, context.clipboard_manager->Get().size());
}

TEST_F(PasteExecutorTest, PasteInto) {
    auto cmd = ParseCommand<PasteCommand>(
        "PasteCommand", R"(parent_name: "Hull_0")");

    PasteExecutor exec;
    auto context = InitContext(exec);

    // Create a HullModel with a BoxModel operand Model.
    auto opbox  = Model::CreateModel<BoxModel>("OpBox");
    auto hull   = Model::CreateModel<HullModel>("Hull_0");
    hull->SetOperandModels(std::vector<ModelPtr>{ opbox });
    context.root_model->AddChildModel(hull);

    // Create a couple of BoxModel instances and store copies in the
    // ClipboardManager.
    auto box0 = Model::CreateModel<BoxModel>("Box_0");
    auto box1 = Model::CreateModel<BoxModel>("Box_1");
    context.clipboard_manager->StoreCopies(std::vector<ModelPtr>{ box0, box1 });

    EXPECT_EQ(1U,    context.root_model->GetChildModelCount());
    EXPECT_EQ(hull,  context.root_model->GetChildModel(0));
    EXPECT_EQ(1U,    hull->GetOperandModels().size());
    EXPECT_EQ(opbox, hull->GetOperandModels()[0]);
    EXPECT_EQ(2U,    context.clipboard_manager->Get().size());

    // Paste them into the HullModel. Should not affect the ClipboardManager or
    // top-level Models.
    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U,        context.root_model->GetChildModelCount());
    EXPECT_EQ(hull,      context.root_model->GetChildModel(0));
    EXPECT_EQ(3U,        hull->GetOperandModels().size());
    EXPECT_EQ(opbox,     hull->GetOperandModels()[0]);
    EXPECT_EQ("Box_0_A", hull->GetOperandModels()[1]->GetName());
    EXPECT_EQ("Box_1_A", hull->GetOperandModels()[2]->GetName());
    EXPECT_EQ(2U,        context.clipboard_manager->Get().size());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(1U,    context.root_model->GetChildModelCount());
    EXPECT_EQ(hull,  context.root_model->GetChildModel(0));
    EXPECT_EQ(1U,    hull->GetOperandModels().size());
    EXPECT_EQ(opbox, hull->GetOperandModels()[0]);
    EXPECT_EQ(2U,    context.clipboard_manager->Get().size());
}

TEST_F(PasteExecutorTest, PasteHierarchy) {
    auto cmd = ParseCommand<PasteCommand>("PasteCommand", "");

    PasteExecutor exec;
    auto context = InitContext(exec);

    // Create a HullModel with a BoxModel operand Model and store a copy in the
    // ClipboardManager.
    auto opbox  = Model::CreateModel<BoxModel>("OpBox");
    auto hull   = Model::CreateModel<HullModel>("Hull_0");
    hull->SetOperandModels(std::vector<ModelPtr>{ opbox });
    EXPECT_EQ(1U,         hull->GetOperandModels().size());
    EXPECT_EQ("OpBox",    hull->GetOperandModels()[0]->GetName());
    context.clipboard_manager->StoreCopies(std::vector<ModelPtr>{ hull });

    EXPECT_EQ(0U, context.root_model->GetChildModelCount());
    EXPECT_EQ(1U, context.clipboard_manager->Get().size());

    // Paste it as a top-level Model. Should not affect the ClipboardManager.
    exec.Execute(*cmd, Command::Op::kDo);
    EXPECT_EQ(1U,         context.root_model->GetChildModelCount());
    auto hcopy = std::dynamic_pointer_cast<HullModel>(
        context.root_model->GetChildModel(0));
    EXPECT_NOT_NULL(hcopy);
    EXPECT_EQ("Hull_0_A", hcopy->GetName());
    for (const auto &m: hcopy->GetOperandModels())
        std::cerr << "XXXX     ******** " << m->GetDesc() << "\n";


    EXPECT_EQ(1U,         hcopy->GetOperandModels().size());
    EXPECT_EQ("OpBox_A",  hcopy->GetOperandModels()[0]->GetName());
    EXPECT_EQ(1U,         context.clipboard_manager->Get().size());

    exec.Execute(*cmd, Command::Op::kUndo);
    EXPECT_EQ(0U, context.root_model->GetChildModelCount());
    EXPECT_EQ(1U, context.clipboard_manager->Get().size());
}
