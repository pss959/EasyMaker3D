#include "Executors/CreateExtrudedExecutor.h"

#include "Commands/CreateExtrudedModelCommand.h"
#include "Models/ExtrudedModel.h"
#include "Util/Tuning.h"

ModelPtr CreateExtrudedExecutor::CreateModel(Command &command) {
    CreateExtrudedModelCommand &cec =
        GetTypedCommand<CreateExtrudedModelCommand>(command);

    std::string name = cec.GetResultName();
    if (name.empty()) {
        name = CreateUniqueName("Extruded");
        cec.SetResultName(name);
    }

    cec.SetInitialScale(TK::kExtrudedModelHeight);

    // Create and initialize the Model.
    ExtrudedModelPtr em = Model::CreateModel<ExtrudedModel>(name);

    InitModelTransform(*em, cec);
    AddModelInteraction(*em);
    SetRandomModelColor(*em);

    // If the Model was not read from a file, drop it from above.
    if (! cec.IsValidating())
        AnimateModelPlacement(*em);

    return em;
}
