#include "Executors/CreateHullExecutor.h"

#include "Commands/CreateHullModelCommand.h"
#include "Models/HullModel.h"

CombinedModelPtr CreateHullExecutor::CreateCombinedModel(Command &command) {
    CreateHullModelCommand &cc =
        GetTypedCommand<CreateHullModelCommand>(command);

    std::string name = cc.GetResultName();
    if (name.empty()) {
        name = CreateUniqueName("Hull");
        cc.SetResultName(name);
    }

    return Model::CreateModel<HullModel>(name);
}
