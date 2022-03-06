#include "Executors/CreateCSGExecutor.h"

#include "Commands/CreateCSGModelCommand.h"
#include "Models/CSGModel.h"

CombinedModelPtr CreateCSGExecutor::CreateCombinedModel(Command &command) {
    CreateCSGModelCommand &cc = GetTypedCommand<CreateCSGModelCommand>(command);

    const CSGOperation op = cc.GetOperation();
    std::string name = cc.GetResultName();
    if (name.empty()) {
        name = CreateUniqueName(Util::EnumToWord(op));
        cc.SetResultName(name);
    }

    CSGModelPtr csg = Model::CreateModel<CSGModel>(name);
    csg->SetOperation(op);

    return csg;
}
