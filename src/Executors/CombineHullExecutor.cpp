#include "Executors/CombineHullExecutor.h"

#include "Commands/CombineHullModelCommand.h"
#include "Models/HullModel.h"

CombinedModelPtr CombineHullExecutor::CreateCombinedModel(
    Command &command, const std::string &name) {
    return Model::CreateModel<HullModel>(name.empty() ?
                                         CreateUniqueName("Hull") : name);
}
