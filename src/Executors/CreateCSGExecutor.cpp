#include "Executors/CreateCSGExecutor.h"

#include "Commands/CreateCSGModelCommand.h"
#include "Models/CSGModel.h"

CombinedModelPtr CreateCSGExecutor::CreateCombinedModel(Command &command) {
    CreateCSGModelCommand &cc = GetTypedCommand<CreateCSGModelCommand>(command);

    const CSGOperation op = cc.GetOperation();
    CSGModelPtr csg =
        Model::CreateModel<CSGModel>(CreateUniqueName(Util::EnumToWord(op)));
    csg->SetOperation(op);

    return csg;
}
