#include "Executors/CreatePrimitiveExecutor.h"

#include "Commands/CreatePrimitiveModelCommand.h"
#include "Models/BoxModel.h"
#include "Models/CylinderModel.h"
#include "Models/SphereModel.h"
#include "Models/TorusModel.h"
#include "Util/General.h"
#include "Util/Tuning.h"

ModelPtr CreatePrimitiveExecutor::CreateModel(Command &command) {
    CreatePrimitiveModelCommand &cc =
        GetTypedCommand<CreatePrimitiveModelCommand>(command);

    std::string name = cc.GetResultName();
    if (name.empty()) {
        name = CreateUniqueName(Util::EnumToWord(cc.GetType()));
        cc.SetResultName(name);
    }

    cc.SetInitialScale(TK::kInitialModelScale);

    // Create and initialize the Model.
    PrimitiveModelPtr pm;
    switch (cc.GetType()) {
      case PrimitiveType::kBox:
        pm = Model::CreateModel<BoxModel>(name);
        break;
      case PrimitiveType::kCylinder:
        pm = Model::CreateModel<CylinderModel>(name);
        break;
      case PrimitiveType::kSphere:
        pm = Model::CreateModel<SphereModel>(name);
        break;
      case PrimitiveType::kTorus:
        pm = Model::CreateModel<TorusModel>(name);
        break;
      default:
        ASSERTM(false, "Bad Primitive type");
    }

    return pm;
}
