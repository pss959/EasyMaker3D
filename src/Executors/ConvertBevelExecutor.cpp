#include "Executors/ConvertBevelExecutor.h"

#include "Commands/ConvertBevelCommand.h"
#include "Math/Bevel.h"
#include "Models/BeveledModel.h"
#include "Util/General.h"

ConvertedModelPtr ConvertBevelExecutor::ConvertModel(
    const ModelPtr &model, const ConvertedModelPtr &primary) {
    BeveledModelPtr beveled_model = Util::CastToDerived<BeveledModel>(model);

    // If the Model isn't already a BeveledModel, create a new one.
    if (! beveled_model) {
        beveled_model =
            Model::CreateModel<BeveledModel>(CreateUniqueName("Beveled"));
        beveled_model->SetOriginalModel(model);

        // Use default Bevel profile unless there is a primary to copy from.
        if (BeveledModelPtr pb = Util::CastToDerived<BeveledModel>(primary))
            beveled_model->SetBevel(pb->GetBevel());
    }

    return beveled_model;
}
