#include "Executors/ConvertClipExecutor.h"

#include "Models/ClippedModel.h"

ConvertedModelPtr ConvertClipExecutor::CreateConvertedModel() {
    return Model::CreateModel<ClippedModel>(CreateUniqueName("Clipped"));
}
