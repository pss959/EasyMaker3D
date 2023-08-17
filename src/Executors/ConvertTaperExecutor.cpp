#include "Executors/ConvertTaperExecutor.h"

#include "Models/TaperedModel.h"

ConvertedModelPtr ConvertTaperExecutor::CreateConvertedModel(
    const Str &name) {
    return Model::CreateModel<TaperedModel>(
        name.empty() ? CreateUniqueName("Tapered") : name);
}
