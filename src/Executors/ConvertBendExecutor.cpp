#include "Executors/ConvertBendExecutor.h"

#include "Models/BentModel.h"

ConvertedModelPtr ConvertBendExecutor::CreateConvertedModel(const Str &name) {
    return Model::CreateModel<BentModel>(
        name.empty() ? CreateUniqueName("Bent") : name);
}
