#include "Executors/CreateCSGExecutor.h"

#include "Commands/CreateCSGModelCommand.h"
#include "Models/CSGModel.h"

CombinedModelPtr CreateCSGExecutor::CreateCombinedModel(Command &command) {
    CreateCSGModelCommand &cc = GetTypedCommand<CreateCSGModelCommand>(command);

    const CSGOperation op = cc.GetOperation();

    const std::string prefix = Util::EnumToWord(op);
    const std::string name   = GetContext().name_manager->Create(prefix);
    CSGModelPtr csg = Model::CreateModel<CSGModel>(name);
    csg->SetOperation(op);

    return csg;
}
