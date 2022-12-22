#include "Tools/ImportTool.h"

#include "Commands/ChangeImportedModelCommand.h"
#include "Items/Settings.h"
#include "Managers/CommandManager.h"
#include "Managers/SettingsManager.h"
#include "Models/ImportedModel.h"
#include "Panels/ImportToolPanel.h"
#include "Util/Assert.h"
#include "Util/General.h"

bool ImportTool::CanAttach(const Selection &sel) const {
    // There has to be exactly one selected ImportedModel.
    return sel.GetCount() == 1U && AreSelectedModelsOfType<ImportedModel>(sel);
}

void ImportTool::InitPanel() {
    auto model = Util::CastToDerived<ImportedModel>(GetModelAttachedTo());
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
    auto model = Util::CastToDerived<ImportedModel>(GetModelAttachedTo());
    ASSERT(model);

    bool is_done = false;

    if (key == "Cancel") {
        is_done = true;
    }

    else if (key == "Accept") {
        auto &panel = GetTypedPanel<ImportToolPanel>();
        const std::string &path = panel.GetPath().ToString();

        auto cimc = CreateCommand<ChangeImportedModelCommand>();
        cimc->SetFromSelection(GetSelection());
        cimc->SetNewPath(path);
        context.command_manager->AddAndDo(cimc);

        // Report any errors.
        std::string reason;
        if (! model->IsMeshValid(reason)) {
            panel.DisplayImportError(reason);
            is_done = false;
        }

        is_done = true;
    }

    else {
        PanelTool::PanelChanged(key, type);
    }

    if (is_done)
        Finish();
}
