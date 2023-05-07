#include "Executors/ConvertTaperExecutor.h"

#include "Models/TaperedModel.h"

ConvertedModelPtr ConvertTaperExecutor::CreateConvertedModel(
    const std::string &name) {
    return Model::CreateModel<TaperedModel>(
        name.empty() ? CreateUniqueName("Tapered") : name);
}
