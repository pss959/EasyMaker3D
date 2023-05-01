#include "Executors/CreateHullExecutor.h"

#include "Commands/CreateHullModelCommand.h"
#include "Models/HullModel.h"

CombinedModelPtr CreateHullExecutor::CreateCombinedModel(
    Command &command, const std::string &name) {
    return Model::CreateModel<HullModel>(name.empty() ?
                                         CreateUniqueName("Hull") : name);
}
