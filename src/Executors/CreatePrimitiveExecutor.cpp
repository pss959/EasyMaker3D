#include "Executors/CreatePrimitiveExecutor.h"

#include "Commands/CreatePrimitiveModelCommand.h"
#include "Models/BoxModel.h"
#include "Models/CylinderModel.h"
#include "Models/SphereModel.h"
#include "Models/TorusModel.h"
#include "Util/General.h"

ModelPtr CreatePrimitiveExecutor::CreateModel(Command &command) {
    ASSERT(dynamic_cast<CreatePrimitiveModelCommand *>(&command));
    CreatePrimitiveModelCommand &cc =
        static_cast<CreatePrimitiveModelCommand &>(command);

    // Create and initialize the Model.
    PrimitiveModelPtr pm;
    std::string prefix;
    switch (cc.GetType()) {
      case PrimitiveType::kBox:
        pm = CREATE_MODEL(BoxModel);
        prefix = "Box";
        break;
      case PrimitiveType::kCylinder:
        pm = CREATE_MODEL(CylinderModel);
        prefix = "Cylinder";
        break;
      case PrimitiveType::kSphere:
        pm = CREATE_MODEL(SphereModel);
        prefix = "Sphere";
        break;
      case PrimitiveType::kTorus:
        pm = CREATE_MODEL(TorusModel);
        prefix = "Torus";
        break;
    }
    pm->ChangeName(GetContext().name_manager->Create(prefix));
    InitModelTransform(*pm, 4);
    AddClickToModel(*pm);
    SetRandomModelColor(*pm);

    // If the Model was not read from a file, drop it from above.
    if (cc.GetModelName().empty()) {
        cc.SetModelName(pm->GetName());
        AnimateModelPlacement(*pm);
    }
    return pm;
}
