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

    return Model::CreateModel<ExtrudedModel>(name);
}
