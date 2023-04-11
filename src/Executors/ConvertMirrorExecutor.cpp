#include "Executors/ConvertMirrorExecutor.h"

#include "Models/MirroredModel.h"

ConvertedModelPtr ConvertMirrorExecutor::CreateConvertedModel() {
    return Model::CreateModel<MirroredModel>(CreateUniqueName("Mirrored"));
}
