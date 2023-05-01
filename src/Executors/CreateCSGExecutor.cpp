#include "Executors/CreateCSGExecutor.h"

#include "Commands/CreateCSGModelCommand.h"
#include "Models/CSGModel.h"

CombinedModelPtr CreateCSGExecutor::CreateCombinedModel(
    Command &command, const std::string &name) {
    CreateCSGModelCommand &cc = GetTypedCommand<CreateCSGModelCommand>(command);
    const CSGOperation op = cc.GetOperation();

    CSGModelPtr csg = Model::CreateModel<CSGModel>(
        name.empty() ? CreateUniqueName(Util::EnumToWord(op)) : name);
    csg->SetOperation(op);

    return csg;
}
