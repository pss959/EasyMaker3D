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

    // Create a dummy ImportedModel. Leave the initial scale at its default
    // value (1).
    ImportedModelPtr im = Model::CreateModel<ImportedModel>(name);
    const auto &settings = GetContext().settings_manager->GetSettings();
    im->SetUnitConversion(settings.GetImportUnitsConversion());

    InitModelTransform(*im, cic);
    AddModelInteraction(*im);

    // If the Model was not read from a file, drop it from above.
    if (! cic.IsValidating())
        AnimateModelPlacement(*im);

    return im;
}
