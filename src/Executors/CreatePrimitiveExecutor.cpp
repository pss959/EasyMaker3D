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
        pm = Model::CreateModel<BoxModel>();
        prefix = "Box";
        break;
      case PrimitiveType::kCylinder:
        pm = Model::CreateModel<CylinderModel>();
        prefix = "Cylinder";
        break;
      case PrimitiveType::kSphere:
        pm = Model::CreateModel<SphereModel>();
        prefix = "Sphere";
        break;
      case PrimitiveType::kTorus:
        pm = Model::CreateModel<TorusModel>();
        prefix = "Torus";
        break;
      default:
        ASSERTM(false, "Bad Primitive type");
    }
    pm->ChangeName(GetContext().name_manager->Create(prefix));
    InitModelTransform(*pm, 4);
    AddModelInteraction(*pm);
    SetRandomModelColor(*pm);

    // If the Model was not read from a file, drop it from above.
    if (cc.GetModelName().empty()) {
        cc.SetModelName(pm->GetName());
        AnimateModelPlacement(*pm);
    }
    return pm;
}
