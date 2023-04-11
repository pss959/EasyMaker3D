#include "Executors/ConvertBevelExecutor.h"

#include "Models/BeveledModel.h"

ConvertedModelPtr ConvertBevelExecutor::CreateConvertedModel() {
    return Model::CreateModel<BeveledModel>(CreateUniqueName("Beveled"));
}
