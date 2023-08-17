#include "Executors/CreateExtrudedExecutor.h"

#include "Commands/CreateExtrudedModelCommand.h"
#include "Models/ExtrudedModel.h"

ModelPtr CreateExtrudedExecutor::CreateModel(Command &command) {
    CreateExtrudedModelCommand &cec =
        GetTypedCommand<CreateExtrudedModelCommand>(command);

    Str name = cec.GetResultName();
    if (name.empty()) {
        name = CreateUniqueName("Extruded");
        cec.SetResultName(name);
    }

    return Model::CreateModel<ExtrudedModel>(name);
}
