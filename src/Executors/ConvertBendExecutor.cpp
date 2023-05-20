#include "Executors/ConvertBendExecutor.h"

#include "Models/BentModel.h"

ConvertedModelPtr ConvertBendExecutor::CreateConvertedModel(
    const std::string &name) {
    return Model::CreateModel<BentModel>(
        name.empty() ? CreateUniqueName("Bent") : name);
}
