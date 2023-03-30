#include "Executors/CreateRevSurfExecutor.h"

#include "Commands/CreateRevSurfModelCommand.h"
#include "Models/RevSurfModel.h"
#include "Util/Tuning.h"

ModelPtr CreateRevSurfExecutor::CreateModel(Command &command) {
    CreateRevSurfModelCommand &crc =
        GetTypedCommand<CreateRevSurfModelCommand>(command);

    std::string name = crc.GetResultName();
    if (name.empty()) {
        name = CreateUniqueName("RevSurf");
        crc.SetResultName(name);
    }

    crc.SetInitialScale(TK::kRevSurfHeight);

    RevSurfModelPtr rsm = Model::CreateModel<RevSurfModel>(name);

    return rsm;
}
