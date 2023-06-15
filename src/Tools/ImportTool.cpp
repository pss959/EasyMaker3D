#include "Tools/ImportTool.h"

#include "Commands/ChangeImportedModelCommand.h"
#include "Commands/CreateImportedModelCommand.h"
#include "Items/Settings.h"
#include "Managers/CommandManager.h"
#include "Managers/SettingsManager.h"
#include "Models/ImportedModel.h"
#include "Panels/ImportToolPanel.h"
#include "Util/Assert.h"

bool ImportTool::CanAttach(const Selection &sel) const {
    // There has to be exactly one selected ImportedModel.
    return sel.GetCount() == 1U && AreSelectedModelsOfType<ImportedModel>(sel);
}

void ImportTool::InitPanel() {
    auto model = std::dynamic_pointer_cast<ImportedModel>(GetModelAttachedTo());
    ASSERT(model);

    auto &panel = GetTypedPanel<ImportToolPanel>();

    const FilePath cur_path = model->GetPath();

    FilePath initial_path = cur_path ? cur_path :
        last_import_path_ ? last_import_path_ :
        GetContext().settings_manager->GetSettings().GetImportDirectory();

    panel.SetTitle("Select an STL file (.stl) to import");
    panel.SetTargetType(FilePanel::TargetType::kExistingFile);
    panel.SetInitialPath(initial_path);
    panel.SetExtension(".stl");
    panel.SetHighlightPath(cur_path, " [CURRENT PATH]");
}

void ImportTool::PanelChanged(const std::string &key,
                              ToolPanel::InteractionType type) {
    const Context &context = GetContext();
    auto model = std::dynamic_pointer_cast<ImportedModel>(GetModelAttachedTo());
    ASSERT(model);

    // Assume the panel is no longer needed.
    bool is_done = true;

    // If the ImportedModel was never loaded successfully, then this tool was
    // used to set up the initial import path.
    const bool is_initial_import = ! model->WasLoadedSuccessfully();

    if (key == "Cancel") {
        // If setting the initial path for the ImportedModel was canceled, the
        // ImportedModel has to be removed by undoing and removing the command
        // that created it.
        if (is_initial_import)
            context.command_manager->UndoAndPurge();

        // Canceling a change to an existing Model does nothing but close the
        // ImportToolPanel.
    }

    else if (key == "Accept") {
        auto &panel = GetTypedPanel<ImportToolPanel>();
        const std::string &path = panel.GetPath().ToString();

        if (is_initial_import) {
            // Accepting the initial path means officially creating the
            // ImportedModel with the correct path. Update the Command and the
            // ImportedModel.
            const auto &cimc =
                std::dynamic_pointer_cast<CreateImportedModelCommand>(
                    context.command_manager->GetLastCommand());
            ASSERT(cimc);
            cimc->SetPath(path);
            model->SetPath(path);
        }
        else {
            // This is a change to an existing ImportedModel.
            auto cimc = CreateCommand<ChangeImportedModelCommand>();
            cimc->SetFromSelection(GetSelection());
            cimc->SetNewPath(path);
            context.command_manager->AddAndDo(cimc);
        }

        // Report any import errors and leave the panel open.
        if (! model->GetErrorMessage().empty()) {
            panel.DisplayImportError("Error importing STL data:\n" +
                                     model->GetErrorMessage());
            is_done = false;
        }
    }

    else {
        PanelTool::PanelChanged(key, type);
    }

    if (is_done)
        Finish();
}
