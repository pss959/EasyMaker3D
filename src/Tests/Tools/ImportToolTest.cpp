#include "Commands/ChangeImportedModelCommand.h"
#include "Commands/CreateImportedModelCommand.h"
#include "Managers/CommandManager.h"
#include "Models/ImportedModel.h"
#include "Models/RootModel.h"
#include "Tools/ImportTool.h"
#include "Tests/Panels/PanelInteractor.h"
#include "Tests/Tools/ToolTestBase.h"
#include "Tests/Testing.h"
#include "Tests/Util/FakeFileSystem.h"

DECL_SHARED_PTR(ImportTool);

/// \ingroup Tests
class ImportToolTest : public ToolTestBase {
  protected:
    ImportToolPtr    tool;
    ImportedModelPtr model;   ///< ImportedModel the Tool is attached to.

    /// The constructor sets up an ImportTool attached to a selected
    /// ImportedModel.
    ImportToolTest();

    /// The destructor detaches the ImportTool.
    ~ImportToolTest() { tool->DetachFromSelection(); }
};

ImportToolTest::ImportToolTest() {
    // Initialize the ImportTool.
    tool = InitTool<ImportTool>("ImportTool");
    EXPECT_TRUE(tool->IsSpecialized());

    // Create and add an ImportedModel.
    model = Model::CreateModel<ImportedModel>("Imported");
    context->root_model->AddChildModel(model);

    // Attach the ImportTool to the ImportedModel.
    Selection sel(SelPath(context->root_model, model));
    EXPECT_TRUE(tool->CanBeUsedFor(sel));
    tool->AttachToSelection(sel, 0);

    EXPECT_NOT_NULL(tool->GetModelAttachedTo());

    AddDummyCommandFunction("CreateImportedModelCommand");

    // Need to modify the ImportedModel for some of this to work.
    auto update_model = [&](const Command &cmd){
        const auto &c = dynamic_cast<const ChangeImportedModelCommand &>(cmd);
        model->SetPath(c.GetNewPath());
        model->GetMesh();  // So errors are detected.
    };
    AddCommandFunction("ChangeImportedModelCommand", update_model);
}

TEST_F(ImportToolTest, Accept) {
    // Have to use the real FileSystem with a valid STL file for this to work
    // without popping up a DialogPanel.
    UseRealFileSystem(true);
    const Str import_path0 = GetDataPath("box.stl").ToString();
    const Str import_path1 = GetDataPath("binarybox.stl").ToString();
    const Str import_path2 = GetDataPath("nomesh.stl").ToString();

    // Accepting a change to the initial import causes the path in the previous
    // CreateImportedModelCommand to be modified, so add it first.
    auto cimc = Command::CreateCommand<CreateImportedModelCommand>();
    context->command_manager->AddAndDo(cimc);

    // Change the path and hit the "Accept" button. Should not add a command,
    // but should update the CreateImportedModelCommand.
    PanelInteractor pi(tool->GetPanel());
    pi.SetTextInput("Input", import_path0);
    EXPECT_TRUE(pi.IsButtonPaneEnabled("Accept"));
    pi.ClickButtonPane("Accept");

    const auto &cmd0 = CheckOneCommand<CreateImportedModelCommand>();
    EXPECT_EQ(import_path0, cmd0.GetPath());

    // Changing the path should cause a ChangeImportedModelCommand to be added.
    pi.SetTextInput("Input", import_path1);
    EXPECT_TRUE(pi.IsButtonPaneEnabled("Accept"));
    pi.ClickButtonPane("Accept");

    const auto &cmd1 = CheckLastCommand<ChangeImportedModelCommand>(2U);
    EXPECT_EQ(import_path1, cmd1.GetNewPath());

    // Changing the path to a bad file should also add a command and should
    // display an error dialog.
    pi.SetTextInput("Input", import_path2);  // Not a closed solid.
    EXPECT_TRUE(pi.IsButtonPaneEnabled("Accept"));
    pi.ClickButtonPane("Accept");

    const auto &cmd2 = CheckLastCommand<ChangeImportedModelCommand>(3U);
    EXPECT_EQ(import_path2, cmd2.GetNewPath());
}

TEST_F(ImportToolTest, Cancel) {
    // Canceling a change to the initial import causes the
    // CreateImportedModelCommand to be undone and purged, so add it first so
    // it can be undone.
    auto cimc = Command::CreateCommand<CreateImportedModelCommand>();
    context->command_manager->AddAndDo(cimc);

    // Change the path and hit the "Cancel" button. Should not add a command.
    PanelInteractor pi(tool->GetPanel());
    pi.SetTextInput("Input", "/a/b/c/test.stl");  // Does not matter.
    pi.ClickButtonPane("Cancel");

    // The CreateImportedModelCommand should have been undone and purged.
    CheckNoCommands();
}
