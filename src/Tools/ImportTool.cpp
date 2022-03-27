#include "Tools/ImportTool.h"

#include "Commands/ChangeImportedModelCommand.h"
#include "Managers/CommandManager.h"
#include "Managers/SettingsManager.h"
#include "Models/ImportedModel.h"
#include "Panels/FilePanel.h"
#include "Settings.h"
#include "Util/Assert.h"
#include "Util/General.h"

bool ImportTool::CanAttach(const Selection &sel) const {
    // There has to be exactly one selected ImportedModel.
    return sel.GetCount() == 1U && AreSelectedModelsOfType<ImportedModel>(sel);
}

void ImportTool::InitPanel() {
    auto im = Util::CastToDerived<ImportedModel>(GetModelAttachedTo());
    ASSERT(im);

    auto &panel = GetTypedPanel<FilePanel>();

    const FilePath cur_path = im->GetPath();

    FilePath initial_path = cur_path ? cur_path :
        last_import_path_ ? last_import_path_ :
        GetContext().settings_manager->GetSettings().GetImportDirectory();

    panel.SetTitle("Select an STL file (.stl) to import");
    panel.SetTargetType(FilePanel::TargetType::kExistingFile);
    panel.SetInitialPath(initial_path);
    panel.SetExtension(".stl");
    panel.SetHighlightPath(cur_path, " [CURRENT PATH]");
}

// XXXX Set up responses.
