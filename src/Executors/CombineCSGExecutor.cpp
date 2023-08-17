#include "Executors/CombineCSGExecutor.h"

#include "Commands/CombineCSGModelCommand.h"
#include "Models/CSGModel.h"

CombinedModelPtr CombineCSGExecutor::CreateCombinedModel(
    Command &command, const Str &name) {
    CombineCSGModelCommand &cc =
        GetTypedCommand<CombineCSGModelCommand>(command);
    const CSGOperation op = cc.GetOperation();

    CSGModelPtr csg = Model::CreateModel<CSGModel>(
        name.empty() ? CreateUniqueName(Util::EnumToWord(op)) : name);
    csg->SetOperation(op);

    return csg;
}
