#include "Executors/ConvertClipExecutor.h"

#include "Commands/ConvertClipCommand.h"
#include "Models/ClippedModel.h"
#include "Util/General.h"

ConvertedModelPtr ConvertClipExecutor::ConvertModel(
    const ModelPtr &model, const ConvertedModelPtr &primary) {
    ClippedModelPtr clipped_model = Util::CastToDerived<ClippedModel>(model);

    // If the Model isn't already a ClippedModel, create a new one.
    if (! clipped_model) {
        clipped_model =
            Model::CreateModel<ClippedModel>(CreateUniqueName("Clipped"));
        clipped_model->SetOriginalModel(model);
    }

    return clipped_model;
}
