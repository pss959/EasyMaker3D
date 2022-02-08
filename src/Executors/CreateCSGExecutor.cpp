#include "Executors/CreateCSGExecutor.h"

#include "Commands/CreateCSGModelCommand.h"
#include "Models/CSGModel.h"

CombinedModelPtr CreateCSGExecutor::CreateCombinedModel(Command &command) {
    ASSERT(dynamic_cast<CreateCSGModelCommand *>(&command));
    CreateCSGModelCommand &cc = static_cast<CreateCSGModelCommand &>(command);

    const CSGOperation op = cc.GetOperation();

    CSGModelPtr csg = Model::CreateModel<CSGModel>();
    csg->SetOperation(op);

    const std::string prefix = Util::EnumToWord(op);
    csg->ChangeName(GetContext().name_manager->Create(prefix));

    AddClickToModel(*csg);
    SetRandomModelColor(*csg);

    return csg;
}
