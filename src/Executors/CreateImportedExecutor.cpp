#include "Executors/CreateImportedExecutor.h"

#include "Commands/CreateImportedModelCommand.h"
#include "Models/ImportedModel.h"

ModelPtr CreateImportedExecutor::CreateModel(Command &command) {
    CreateImportedModelCommand &cic =
        GetTypedCommand<CreateImportedModelCommand>(command);

    std::string name = cic.GetResultName();
    if (name.empty()) {
        name = CreateUniqueName("Imported");
        cic.SetResultName(name);
    }

    // Create and initialize the Model.
    ImportedModelPtr im = Model::CreateModel<ImportedModel>(name);
    InitModelTransform(*im, 1);
    AddModelInteraction(*im);
    SetRandomModelColor(*im);

    // If the Model was not read from a file, drop it from above.
    if (! cic.IsValidating())
        AnimateModelPlacement(*im);

    return im;
}
