#include "Executors/CreateImportedExecutor.h"

#include "Commands/CreateImportedModelCommand.h"
#include "Managers/SettingsManager.h"
#include "Models/ImportedModel.h"
#include "Items/Settings.h"
#include "Util/Assert.h"

ModelPtr CreateImportedExecutor::CreateModel(Command &command) {
    CreateImportedModelCommand &cic =
        GetTypedCommand<CreateImportedModelCommand>(command);

    std::string name = cic.GetResultName();
    if (name.empty()) {
        name = CreateUniqueName("Imported");
        cic.SetResultName(name);
    }

    // Create and initialize the Model. Leave the initial scale at its default
    // value (1).
    ImportedModelPtr im = Model::CreateModel<ImportedModel>(name);
    const auto &settings = GetContext().settings_manager->GetSettings();

    // If the Model was read from a file, the command must have a valid path,
    // so set the path so the ImportedModel can check it.
    if (cic.IsValidating()) {
        im->SetPath(cic.GetPath());
    }
    // Otherwise, the path will be empty until the ImportTool is used to select
    // a real file path, so do not set the path.
    else {
        ASSERT(cic.GetPath().empty());
    }

    im->SetUnitConversion(settings.GetImportUnitsConversion());

    return im;
}
