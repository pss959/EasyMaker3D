#include "Executors/ConvertTwistExecutor.h"

#include "Models/TwistedModel.h"

ConvertedModelPtr ConvertTwistExecutor::CreateConvertedModel(
    const std::string &name) {
    return Model::CreateModel<TwistedModel>(
        name.empty() ? CreateUniqueName("Twisted") : name);
}
