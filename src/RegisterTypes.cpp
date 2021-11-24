#include "RegisterTypes.h"

#include "Commands/CreatePrimitiveModelCommand.h"
#include "Commands/TranslateCommand.h"
#include "Feedback/LinearFeedback.h"
#include "Items/Board.h"
#include "Items/Controller.h"
#include "Items/Frame.h"
#include "Items/GripGuide.h"
#include "Items/Icon.h"
#include "Items/PaneBackground.h"
#include "Items/PaneBorder.h"
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
#include "Panels/FilePanel.h"
#include "Panels/HelpPanel.h"
#include "Panels/SessionPanel.h"
#include "Panels/SettingsPanel.h"
#include "Panels/TestPanel.h"
#include "Panes/BoxPane.h"
#include "Panes/ButtonPane.h"
#include "Panes/GridPane.h"
#include "Panes/ImagePane.h"
#include "Panes/SpacerPane.h"
#include "Panes/TextInputPane.h"
#include "Panes/TextPane.h"
#include "Parser/Registry.h"
#include "SG/Box.h"
#include "SG/Cylinder.h"
#include "SG/Ellipsoid.h"
#include "SG/FileImage.h"
#include "SG/Gantry.h"
#include "SG/ImportedShape.h"
#include "SG/Init.h"
#include "SG/LayoutOptions.h"
#include "SG/LightingPass.h"
#include "SG/Line.h"
#include "SG/Material.h"
#include "SG/Node.h"
#include "SG/PointLight.h"
#include "SG/PolyLine.h"
#include "SG/Polygon.h"
#include "SG/ProceduralImage.h"
#include "SG/Rectangle.h"
#include "SG/Sampler.h"
#include "SG/Scene.h"
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
#include "Targets/EdgeTarget.h"
#include "Targets/PointTarget.h"
#include "Tools/TranslationTool.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/IconWidget.h"
#include "Widgets/PushButtonWidget.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/Slider2DWidget.h"

// ----------------------------------------------------------------------------
// Helper macros.
// ----------------------------------------------------------------------------

#define ADD_SG_TYPE_(T) Parser::Registry::AddType<SG::T>(#T)
#define ADD_TYPE_(T)    Parser::Registry::AddType<T>(#T)

// ----------------------------------------------------------------------------
// Helper functions. Each of these registers concrete types for a module.
// ----------------------------------------------------------------------------

static void RegisterCommandTypes_() {
    ADD_TYPE_(CreatePrimitiveModelCommand);
    ADD_TYPE_(TranslateCommand);
}

static void RegisterFeedbackTypes_() {
    ADD_TYPE_(LinearFeedback);
}

static void RegisterItemTypes_() {
    ADD_TYPE_(Board);
    ADD_TYPE_(Controller);
    ADD_TYPE_(Frame);
    ADD_TYPE_(GripGuide);
    ADD_TYPE_(Icon);
    ADD_TYPE_(PaneBackground);
    ADD_TYPE_(PaneBorder);
    ADD_TYPE_(Shelf);
    ADD_TYPE_(Tooltip);
}

static void RegisterModelTypes_() {
    ADD_TYPE_(BeveledModel);
    ADD_TYPE_(BoxModel);
    ADD_TYPE_(CSGModel);
    ADD_TYPE_(CylinderModel);
    ADD_TYPE_(RootModel);
    ADD_TYPE_(RevSurfModel);
    ADD_TYPE_(SphereModel);
    ADD_TYPE_(TorusModel);
}

static void RegisterPaneTypes_() {
    ADD_TYPE_(BoxPane);
    ADD_TYPE_(ButtonPane);
    ADD_TYPE_(GridPane);
    ADD_TYPE_(ImagePane);
    ADD_TYPE_(SpacerPane);
    ADD_TYPE_(TextInputPane);
    ADD_TYPE_(TextPane);
}

static void RegisterPanelTypes_() {
    ADD_TYPE_(FilePanel);
    ADD_TYPE_(HelpPanel);
    ADD_TYPE_(SessionPanel);
    ADD_TYPE_(SettingsPanel);
#if DEBUG
    ADD_TYPE_(TestPanel);
#endif
}

static void RegisterSGTypes_() {
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
    ADD_SG_TYPE_(PolyLine);
    ADD_SG_TYPE_(Polygon);
    ADD_SG_TYPE_(ProceduralImage);
    ADD_SG_TYPE_(Rectangle);
    ADD_SG_TYPE_(Sampler);
    ADD_SG_TYPE_(Scene);
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
}

static void RegisterTargetTypes_() {
    ADD_TYPE_(EdgeTarget);
    ADD_TYPE_(PointTarget);
}

static void RegisterToolTypes_() {
    ADD_TYPE_(TranslationTool);
}

static void RegisterWidgetTypes_() {
    ADD_TYPE_(DiscWidget);
    ADD_TYPE_(IconWidget);
    ADD_TYPE_(PushButtonWidget);
    ADD_TYPE_(Slider1DWidget);
    ADD_TYPE_(Slider2DWidget);
}

// ----------------------------------------------------------------------------

#undef ADD_SG_TYPE_
#undef ADD_TYPE_

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

void RegisterTypes() {
    // Make sure SG is initialized.
    SG::Init();

    RegisterCommandTypes_();
    RegisterFeedbackTypes_();
    RegisterItemTypes_();
    RegisterModelTypes_();
    RegisterPaneTypes_();
    RegisterPanelTypes_();
    RegisterSGTypes_();
    RegisterTargetTypes_();
    RegisterToolTypes_();
    RegisterWidgetTypes_();
}

void UnregisterTypes() {
    Parser::Registry::Clear();
}
