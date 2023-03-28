#include "App/RegisterTypes.h"

#include "Commands/ChangeBevelCommand.h"
#include "Commands/ChangeCSGOperationCommand.h"
#include "Commands/ChangeClipCommand.h"
#include "Commands/ChangeColorCommand.h"
#include "Commands/ChangeComplexityCommand.h"
#include "Commands/ChangeCylinderCommand.h"
#include "Commands/ChangeEdgeTargetCommand.h"
#include "Commands/ChangeImportedModelCommand.h"
#include "Commands/ChangeMirrorCommand.h"
#include "Commands/ChangeNameCommand.h"
#include "Commands/ChangeOrderCommand.h"
#include "Commands/ChangePointTargetCommand.h"
#include "Commands/ChangeRevSurfCommand.h"
#include "Commands/ChangeTextCommand.h"
#include "Commands/ChangeTorusCommand.h"
#include "Commands/CommandList.h"
#include "Commands/ConvertBevelCommand.h"
#include "Commands/ConvertClipCommand.h"
#include "Commands/ConvertMirrorCommand.h"
#include "Commands/CopyCommand.h"
#include "Commands/CreateCSGModelCommand.h"
#include "Commands/CreateHullModelCommand.h"
#include "Commands/CreateImportedModelCommand.h"
#include "Commands/CreatePrimitiveModelCommand.h"
#include "Commands/CreateRevSurfModelCommand.h"
#include "Commands/CreateTextModelCommand.h"
#include "Commands/DeleteCommand.h"
#include "Commands/LinearLayoutCommand.h"
#include "Commands/PasteCommand.h"
#include "Commands/RadialLayoutCommand.h"
#include "Commands/RotateCommand.h"
#include "Commands/ScaleCommand.h"
#include "Commands/TranslateCommand.h"
#include "Feedback/AngularFeedback.h"
#include "Feedback/LinearFeedback.h"
#include "Feedback/TooltipFeedback.h"
#include "Items/AppInfo.h"
#include "Items/Border.h"
#include "Items/BuildVolume.h"
#include "Items/Controller.h"
#include "Items/Frame.h"
#include "Items/GripGuide.h"
#include "Items/Inspector.h"
#include "Items/PaneBackground.h"
#include "Items/PrecisionControl.h"
#include "Items/RadialMenu.h"
#include "Items/RadialMenuInfo.h"
#include "Items/SessionState.h"
#include "Items/Settings.h"
#include "Items/Shelf.h"
#include "Items/UnitConversion.h"
#include "Models/BeveledModel.h"
#include "Models/BoxModel.h"
#include "Models/CSGModel.h"
#include "Models/ClippedModel.h"
#include "Models/CylinderModel.h"
#include "Models/ExtrudedModel.h"
#include "Models/HullModel.h"
#include "Models/ImportedModel.h"
#include "Models/MirroredModel.h"
#include "Models/RootModel.h"
#include "Models/RevSurfModel.h"
#include "Models/SphereModel.h"
#include "Models/TextModel.h"
#include "Models/TorusModel.h"
#include "Panels/ActionPanel.h"
#include "Panels/BevelToolPanel.h"
#include "Panels/Board.h"
#include "Panels/CSGToolPanel.h"
#include "Panels/DialogPanel.h"
#include "Panels/FilePanel.h"
#include "Panels/HelpPanel.h"
#include "Panels/ImportToolPanel.h"
#include "Panels/InfoPanel.h"
#include "Panels/KeyboardPanel.h"
#include "Panels/NameToolPanel.h"
#include "Panels/RadialMenuPanel.h"
#include "Panels/RevSurfToolPanel.h"
#include "Panels/SessionPanel.h"
#include "Panels/SettingsPanel.h"
#include "Panels/TestPanel.h"
#include "Panels/TextToolPanel.h"
#include "Panels/TreePanel.h"
#include "Panes/BoxPane.h"
#include "Panes/ButtonPane.h"
#include "Panes/CheckboxPane.h"
#include "Panes/ClipPane.h"
#include "Panes/DropdownPane.h"
#include "Panes/GridPane.h"
#include "Panes/IconPane.h"
#include "Panes/ImagePane.h"
#include "Panes/LabeledSliderPane.h"
#include "Panes/ProfilePane.h"
#include "Panes/RadioButtonPane.h"
#include "Panes/ScrollingPane.h"
#include "Panes/SliderPane.h"
#include "Panes/SpacerPane.h"
#include "Panes/SpecialKeyPane.h"
#include "Panes/SwitcherPane.h"
#include "Panes/TextInputPane.h"
#include "Panes/TextKeyPane.h"
#include "Panes/TextPane.h"
#include "Panes/TouchWrapperPane.h"
#include "Parser/Registry.h"
#include "Place/EdgeTarget.h"
#include "Place/PointTarget.h"
#include "SG/Box.h"
#include "SG/ColorMap.h"
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
#include "SG/MutableTriMeshShape.h"
#include "SG/NamedColor.h"
#include "SG/Node.h"
#include "SG/PointLight.h"
#include "SG/PolyLine.h"
#include "SG/Polygon.h"
#include "SG/ProceduralImage.h"
#include "SG/Rectangle.h"
#include "SG/RegularPolygon.h"
#include "SG/Sampler.h"
#include "SG/Scene.h"
#include "SG/ShaderProgram.h"
#include "SG/ShaderSource.h"
#include "SG/ShadowPass.h"
#include "SG/StateTable.h"
#include "SG/SubImage.h"
#include "SG/TextNode.h"
#include "SG/Texture.h"
#include "SG/Torus.h"
#include "SG/Tube.h"
#include "SG/Uniform.h"
#include "SG/UniformBlock.h"
#include "SG/UniformDef.h"
#include "SG/UnscopedNode.h"
#include "SG/VRCamera.h"
#include "SG/WindowCamera.h"
#include "Tools/BevelTool.h"
#include "Tools/CSGTool.h"
#include "Tools/ClipTool.h"
#include "Tools/ColorTool.h"
#include "Tools/ComplexityTool.h"
#include "Tools/CylinderTool.h"
#include "Tools/ImportTool.h"
#include "Tools/MirrorTool.h"
#include "Tools/NameTool.h"
#include "Tools/PassiveTool.h"
#include "Tools/RevSurfTool.h"
#include "Tools/RotationTool.h"
#include "Tools/ScaleTool.h"
#include "Tools/TextTool.h"
#include "Tools/TorusTool.h"
#include "Tools/TranslationTool.h"
#include "Widgets/DiscWidget.h"
#include "Widgets/EdgeTargetWidget.h"
#include "Widgets/GenericWidget.h"
#include "Widgets/IconSwitcherWidget.h"
#include "Widgets/IconWidget.h"
#include "Widgets/PointTargetWidget.h"
#include "Widgets/PushButtonWidget.h"
#include "Widgets/RadialLayoutWidget.h"
#include "Widgets/ScaleWidget.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/Slider2DWidget.h"
#include "Widgets/SphereWidget.h"
#include "Widgets/StageWidget.h"

// ----------------------------------------------------------------------------
// Helper macros.
// ----------------------------------------------------------------------------

#define ADD_SG_TYPE_(T) Parser::Registry::AddType<SG::T>(#T)
#define ADD_TYPE_(T)    Parser::Registry::AddType<T>(#T)

// ----------------------------------------------------------------------------
// Helper functions. Each of these registers concrete types for a module.
// ----------------------------------------------------------------------------

static void RegisterCommandTypes_() {
    // Types required for command processing.
    ADD_TYPE_(AppInfo);
    ADD_TYPE_(CommandList);
    ADD_TYPE_(SessionState);

    ADD_TYPE_(ChangeBevelCommand);
    ADD_TYPE_(ChangeCSGOperationCommand);
    ADD_TYPE_(ChangeClipCommand);
    ADD_TYPE_(ChangeColorCommand);
    ADD_TYPE_(ChangeComplexityCommand);
    ADD_TYPE_(ChangeCylinderCommand);
    ADD_TYPE_(ChangeEdgeTargetCommand);
    ADD_TYPE_(ChangeImportedModelCommand);
    ADD_TYPE_(ChangeMirrorCommand);
    ADD_TYPE_(ChangeNameCommand);
    ADD_TYPE_(ChangeOrderCommand);
    ADD_TYPE_(ChangePointTargetCommand);
    ADD_TYPE_(ChangeRevSurfCommand);
    ADD_TYPE_(ChangeTextCommand);
    ADD_TYPE_(ChangeTorusCommand);
    ADD_TYPE_(ConvertBevelCommand);
    ADD_TYPE_(ConvertClipCommand);
    ADD_TYPE_(ConvertMirrorCommand);
    ADD_TYPE_(CopyCommand);
    ADD_TYPE_(CreateCSGModelCommand);
    ADD_TYPE_(CreateHullModelCommand);
    ADD_TYPE_(CreateImportedModelCommand);
    ADD_TYPE_(CreatePrimitiveModelCommand);
    ADD_TYPE_(CreateRevSurfModelCommand);
    ADD_TYPE_(CreateTextModelCommand);
    ADD_TYPE_(DeleteCommand);
    ADD_TYPE_(LinearLayoutCommand);
    ADD_TYPE_(PasteCommand);
    ADD_TYPE_(RadialLayoutCommand);
    ADD_TYPE_(RotateCommand);
    ADD_TYPE_(ScaleCommand);
    ADD_TYPE_(TranslateCommand);
}

static void RegisterFeedbackTypes_() {
    ADD_TYPE_(AngularFeedback);
    ADD_TYPE_(LinearFeedback);
    ADD_TYPE_(TooltipFeedback);
}

static void RegisterItemTypes_() {
    ADD_TYPE_(Board);
    ADD_TYPE_(Border);
    ADD_TYPE_(BuildVolume);
    ADD_TYPE_(Controller);
    ADD_TYPE_(Frame);
    ADD_TYPE_(GripGuide);
    ADD_TYPE_(Inspector);
    ADD_TYPE_(PaneBackground);
    ADD_TYPE_(PrecisionControl);
    ADD_TYPE_(RadialMenu);
    ADD_TYPE_(Shelf);
}

static void RegisterModelTypes_() {
    ADD_TYPE_(BeveledModel);
    ADD_TYPE_(BoxModel);
    ADD_TYPE_(CSGModel);
    ADD_TYPE_(ClippedModel);
    ADD_TYPE_(CylinderModel);
    ADD_TYPE_(ExtrudedModel);
    ADD_TYPE_(HullModel);
    ADD_TYPE_(ImportedModel);
    ADD_TYPE_(MirroredModel);
    ADD_TYPE_(RootModel);
    ADD_TYPE_(RevSurfModel);
    ADD_TYPE_(SphereModel);
    ADD_TYPE_(TextModel);
    ADD_TYPE_(TorusModel);
}

static void RegisterPaneTypes_() {
    ADD_TYPE_(BoxPane);
    ADD_TYPE_(ButtonPane);
    ADD_TYPE_(CheckboxPane);
    ADD_TYPE_(ClipPane);
    ADD_TYPE_(DropdownPane);
    ADD_TYPE_(GridPane);
    ADD_TYPE_(IconPane);
    ADD_TYPE_(ImagePane);
    ADD_TYPE_(LabeledSliderPane);
    ADD_TYPE_(ProfilePane);
    ADD_TYPE_(RadioButtonPane);
    ADD_TYPE_(ScrollingPane);
    ADD_TYPE_(SliderPane);
    ADD_TYPE_(SpacerPane);
    ADD_TYPE_(SpecialKeyPane);
    ADD_TYPE_(SwitcherPane);
    ADD_TYPE_(TextInputPane);
    ADD_TYPE_(TextKeyPane);
    ADD_TYPE_(TextPane);
    ADD_TYPE_(TouchWrapperPane);
}

static void RegisterPanelTypes_() {
    ADD_TYPE_(ActionPanel);
    ADD_TYPE_(BevelToolPanel);
    ADD_TYPE_(CSGToolPanel);
    ADD_TYPE_(DialogPanel);
    ADD_TYPE_(FilePanel);
    ADD_TYPE_(HelpPanel);
    ADD_TYPE_(ImportToolPanel);
    ADD_TYPE_(InfoPanel);
    ADD_TYPE_(KeyboardPanel);
    ADD_TYPE_(NameToolPanel);
    ADD_TYPE_(RadialMenuPanel);
    ADD_TYPE_(RevSurfToolPanel);
    ADD_TYPE_(SessionPanel);
    ADD_TYPE_(SettingsPanel);
#if ENABLE_DEBUG_FEATURES
    ADD_TYPE_(TestPanel);
#endif
    ADD_TYPE_(TextToolPanel);
    ADD_TYPE_(TreePanel);
}

static void RegisterSGTypes_() {
    ADD_SG_TYPE_(Box);
    ADD_SG_TYPE_(ColorMap);
    ADD_SG_TYPE_(Cylinder);
    ADD_SG_TYPE_(Ellipsoid);
    ADD_SG_TYPE_(FileImage);
    ADD_SG_TYPE_(Gantry);
    ADD_SG_TYPE_(ImportedShape);
    ADD_SG_TYPE_(LayoutOptions);
    ADD_SG_TYPE_(LightingPass);
    ADD_SG_TYPE_(Line);
    ADD_SG_TYPE_(Material);
    ADD_SG_TYPE_(MutableTriMeshShape);
    ADD_SG_TYPE_(NamedColor);
    ADD_SG_TYPE_(Node);
    ADD_SG_TYPE_(PointLight);
    ADD_SG_TYPE_(PolyLine);
    ADD_SG_TYPE_(Polygon);
    ADD_SG_TYPE_(ProceduralImage);
    ADD_SG_TYPE_(Rectangle);
    ADD_SG_TYPE_(RegularPolygon);
    ADD_SG_TYPE_(Sampler);
    ADD_SG_TYPE_(Scene);
    ADD_SG_TYPE_(ShaderProgram);
    ADD_SG_TYPE_(ShaderSource);
    ADD_SG_TYPE_(ShadowPass);
    ADD_SG_TYPE_(StateTable);
    ADD_SG_TYPE_(SubImage);
    ADD_SG_TYPE_(TextNode);
    ADD_SG_TYPE_(Texture);
    ADD_SG_TYPE_(Torus);
    ADD_SG_TYPE_(Tube);
    ADD_SG_TYPE_(Uniform);
    ADD_SG_TYPE_(UniformBlock);
    ADD_SG_TYPE_(UniformDef);
    ADD_SG_TYPE_(UnscopedNode);
    ADD_SG_TYPE_(VRCamera);
    ADD_SG_TYPE_(WindowCamera);
}

static void RegisterSettingsTypes_() {
    ADD_TYPE_(RadialMenuInfo);
    ADD_TYPE_(Settings);
    ADD_TYPE_(UnitConversion);
}

static void RegisterTargetTypes_() {
    ADD_TYPE_(EdgeTarget);
    ADD_TYPE_(PointTarget);
}

static void RegisterToolTypes_() {
    ADD_TYPE_(BevelTool);
    ADD_TYPE_(CSGTool);
    ADD_TYPE_(ClipTool);
    ADD_TYPE_(ColorTool);
    ADD_TYPE_(ComplexityTool);
    ADD_TYPE_(CylinderTool);
    ADD_TYPE_(ImportTool);
    ADD_TYPE_(MirrorTool);
    ADD_TYPE_(NameTool);
    ADD_TYPE_(PassiveTool);
    ADD_TYPE_(RevSurfTool);
    ADD_TYPE_(RotationTool);
    ADD_TYPE_(ScaleTool);
    ADD_TYPE_(TextTool);
    ADD_TYPE_(TorusTool);
    ADD_TYPE_(TranslationTool);
}

static void RegisterWidgetTypes_() {
    ADD_TYPE_(DiscWidget);
    ADD_TYPE_(EdgeTargetWidget);
    ADD_TYPE_(GenericWidget);
    ADD_TYPE_(IconSwitcherWidget);
    ADD_TYPE_(IconWidget);
    ADD_TYPE_(PointTargetWidget);
    ADD_TYPE_(PushButtonWidget);
    ADD_TYPE_(RadialLayoutWidget);
    ADD_TYPE_(ScaleWidget);
    ADD_TYPE_(Slider1DWidget);
    ADD_TYPE_(Slider2DWidget);
    ADD_TYPE_(SphereWidget);
    ADD_TYPE_(StageWidget);
}

// ----------------------------------------------------------------------------

#undef ADD_SG_TYPE_
#undef ADD_TYPE_

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

void RegisterTypes() {
    // Make it OK to call this more than once (mostly to simplify tests).
    if (Parser::Registry::GetTypeNameCount() > 0)
        return;

    // Make sure SG is initialized.
    SG::Init();

    RegisterCommandTypes_();
    RegisterFeedbackTypes_();
    RegisterItemTypes_();
    RegisterModelTypes_();
    RegisterPaneTypes_();
    RegisterPanelTypes_();
    RegisterSGTypes_();
    RegisterSettingsTypes_();
    RegisterTargetTypes_();
    RegisterToolTypes_();
    RegisterWidgetTypes_();
}

void UnregisterTypes() {
    Parser::Registry::Clear();
}
