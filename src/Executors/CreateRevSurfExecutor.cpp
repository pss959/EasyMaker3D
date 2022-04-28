#include "Executors/CreateRevSurfExecutor.h"

#include "Commands/CreateRevSurfModelCommand.h"
#include "Models/RevSurfModel.h"

ModelPtr CreateRevSurfExecutor::CreateModel(Command &command) {
    CreateRevSurfModelCommand &crc =
        GetTypedCommand<CreateRevSurfModelCommand>(command);

    std::string name = crc.GetResultName();
    if (name.empty()) {
        name = CreateUniqueName("RevSurf");
        crc.SetResultName(name);
    }

    // Create and initialize the Model.
    RevSurfModelPtr rsm = Model::CreateModel<RevSurfModel>(name);

    InitModelTransform(*rsm, 8, crc);
    AddModelInteraction(*rsm);
    SetRandomModelColor(*rsm);

    // If the Model was not read from a file, drop it from above.
    if (! crc.IsValidating())
        AnimateModelPlacement(*rsm);

    return rsm;
}
