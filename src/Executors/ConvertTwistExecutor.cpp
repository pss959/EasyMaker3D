#include "Executors/ConvertTwistExecutor.h"

#include "Commands/ConvertTwistCommand.h"
#include "Models/TwistedModel.h"
#include "Util/General.h"

ConvertedModelPtr ConvertTwistExecutor::ConvertModel(
    const ModelPtr &model, const ConvertedModelPtr &primary) {
    TwistedModelPtr twisted_model = Util::CastToDerived<TwistedModel>(model);

    // If the Model isn't already a TwistedModel, create a new one.
    if (! twisted_model) {
        twisted_model =
            Model::CreateModel<TwistedModel>(CreateUniqueName("Twisted"));
        twisted_model->SetOriginalModel(model);
    }

    return twisted_model;
}
