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
        beveled_model = Model::CreateModel<BeveledModel>();

        // Use default Bevel profile unless there is a primary to copy from.
        if (BeveledModelPtr pbm = Util::CastToDerived<BeveledModel>(primary))
            beveled_model->SetBevel(pbm->GetBevel());
    }

    return beveled_model;
}
