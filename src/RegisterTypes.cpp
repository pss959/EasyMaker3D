#include "RegisterTypes.h"

#include "Commands/CreatePrimitiveModelCommand.h"
#include "Commands/TranslateCommand.h"
#include "Items/Shelf.h"
#include "Items/Tooltip.h"
#include "Models/BeveledModel.h"
#include "Models/BoxModel.h"
#include "Models/CSGModel.h"
#include "Models/CylinderModel.h"
#include "Models/RootModel.h"
#include "Models/RevSurfModel.h"
#include "Models/SphereModel.h"
#include "Models/TorusModel.h"
#include "Parser/Registry.h"
#include "SG/Box.h"
#include "SG/Cylinder.h"
#include "SG/Ellipsoid.h"
#include "SG/FileImage.h"
#include "SG/Gantry.h"
#include "SG/ImportedShape.h"
#include "SG/LayoutOptions.h"
#include "SG/LightingPass.h"
#include "SG/Line.h"
#include "SG/Material.h"
#include "SG/Node.h"
#include "SG/PointLight.h"
#include "SG/Polygon.h"
#include "SG/ProceduralImage.h"
#include "SG/Rectangle.h"
#include "SG/Sampler.h"
#include "SG/Scene.h"
#include "SG/ShaderNode.h"
#include "SG/ShaderProgram.h"
#include "SG/ShaderSource.h"
#include "SG/ShadowPass.h"
#include "SG/StateTable.h"
#include "SG/TextNode.h"
#include "SG/Texture.h"
#include "SG/Torus.h"
#include "SG/Uniform.h"
#include "SG/UniformBlock.h"
#include "SG/UniformDef.h"
#include "SG/VRCamera.h"
#include "SG/WindowCamera.h"
#include "Tools/TranslationTool.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/PushButtonWidget.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/ToggleButtonWidget.h"

void RegisterTypes() {

#define ADD_SG_TYPE_(T) Parser::Registry::AddType<SG::T>(#T)
#define ADD_TYPE_(T)    Parser::Registry::AddType<T>(#T)

    // Concrete SG types.
    ADD_SG_TYPE_(Box);
    ADD_SG_TYPE_(Cylinder);
    ADD_SG_TYPE_(Ellipsoid);
    ADD_SG_TYPE_(FileImage);
    ADD_SG_TYPE_(Gantry);
    ADD_SG_TYPE_(ImportedShape);
    ADD_SG_TYPE_(LayoutOptions);
    ADD_SG_TYPE_(LightingPass);
    ADD_SG_TYPE_(Line);
    ADD_SG_TYPE_(Material);
    ADD_SG_TYPE_(Node);
    ADD_SG_TYPE_(PointLight);
    ADD_SG_TYPE_(Polygon);
    ADD_SG_TYPE_(ProceduralImage);
    ADD_SG_TYPE_(Rectangle);
    ADD_SG_TYPE_(Sampler);
    ADD_SG_TYPE_(Scene);
    ADD_SG_TYPE_(ShaderNode);
    ADD_SG_TYPE_(ShaderProgram);
    ADD_SG_TYPE_(ShaderSource);
    ADD_SG_TYPE_(ShadowPass);
    ADD_SG_TYPE_(StateTable);
    ADD_SG_TYPE_(TextNode);
    ADD_SG_TYPE_(Texture);
    ADD_SG_TYPE_(Torus);
    ADD_SG_TYPE_(Uniform);
    ADD_SG_TYPE_(UniformBlock);
    ADD_SG_TYPE_(UniformDef);
    ADD_SG_TYPE_(VRCamera);
    ADD_SG_TYPE_(WindowCamera);

    // Concrete Widget types.
    ADD_TYPE_(DiscWidget);
    ADD_TYPE_(PushButtonWidget);
    ADD_TYPE_(Slider1DWidget);
    ADD_TYPE_(ToggleButtonWidget);

    // Concrete Model types.
    ADD_TYPE_(BeveledModel);
    ADD_TYPE_(BoxModel);
    ADD_TYPE_(CSGModel);
    ADD_TYPE_(CylinderModel);
    ADD_TYPE_(RootModel);
    ADD_TYPE_(RevSurfModel);
    ADD_TYPE_(SphereModel);
    ADD_TYPE_(TorusModel);

    // Concrete Item types.
    ADD_TYPE_(Shelf);
    ADD_TYPE_(Tooltip);

    // Concrete Command types.
    ADD_TYPE_(CreatePrimitiveModelCommand);
    ADD_TYPE_(TranslateCommand);

    // Concrete Tool types.
    ADD_TYPE_(TranslationTool);

#undef ADD_SG_TYPE_
#undef ADD_TYPE_
}

void UnregisterTypes() {
    Parser::Registry::Clear();
}
