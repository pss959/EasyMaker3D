#include "Executors/ConvertTwistExecutor.h"

#include "Models/TwistedModel.h"

ConvertedModelPtr ConvertTwistExecutor::CreateConvertedModel() {
    return Model::CreateModel<TwistedModel>(CreateUniqueName("Twisted"));
}
