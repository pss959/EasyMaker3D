#include "Executors/ConvertBevelExecutor.h"

#include "Models/BeveledModel.h"

ConvertedModelPtr ConvertBevelExecutor::CreateConvertedModel(
    const std::string &name) {
    return Model::CreateModel<BeveledModel>(
        name.empty() ? CreateUniqueName("Beveled") : name);
}
