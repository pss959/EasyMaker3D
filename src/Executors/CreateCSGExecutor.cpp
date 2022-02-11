#include "Executors/CreateCSGExecutor.h"

#include "Commands/CreateCSGModelCommand.h"
#include "Models/CSGModel.h"

CombinedModelPtr CreateCSGExecutor::CreateCombinedModel(Command &command) {
    CreateCSGModelCommand &cc = GetTypedCommand<CreateCSGModelCommand>(command);

    const CSGOperation op = cc.GetOperation();

    CSGModelPtr csg = Model::CreateModel<CSGModel>();
    csg->SetOperation(op);

    const std::string prefix = Util::EnumToWord(op);
    csg->ChangeModelName(GetContext().name_manager->Create(prefix), false);

    return csg;
}
