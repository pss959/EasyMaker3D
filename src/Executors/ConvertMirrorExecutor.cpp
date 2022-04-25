#include "Executors/ConvertMirrorExecutor.h"

#include "Commands/ConvertMirrorCommand.h"
#include "Models/MirroredModel.h"
#include "Util/General.h"

ConvertedModelPtr ConvertMirrorExecutor::ConvertModel(
    const ModelPtr &model, const ConvertedModelPtr &primary) {
    MirroredModelPtr mirrored_model = Util::CastToDerived<MirroredModel>(model);

    // If the Model isn't already a MirroredModel, create a new one.
    if (! mirrored_model) {
        mirrored_model =
            Model::CreateModel<MirroredModel>(CreateUniqueName("Mirrored"));
        mirrored_model->SetOriginalModel(model);
    }

    return mirrored_model;
}
