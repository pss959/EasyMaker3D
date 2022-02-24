#include "Executors/CreateHullExecutor.h"

#include "Models/HullModel.h"

CombinedModelPtr CreateHullExecutor::CreateCombinedModel(Command &command) {
    return Model::CreateModel<HullModel>(CreateUniqueName("Hull"));
}
