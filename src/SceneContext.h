
#pragma once

#include <memory>

#include "Items/Board.h"
#include "Items/Controller.h"
#include "Items/Inspector.h"
#include "Items/RadialMenu.h"
#include "Math/Types.h"
#include "Models/RootModel.h"
#include "Panels/TreePanel.h"
#include "SG/Gantry.h"
#include "SG/Line.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "SG/Scene.h"
#include "SG/TextNode.h"
#include "SG/VRCamera.h"
#include "SG/WindowCamera.h"
#include "Widgets/StageWidget.h"
#include "Widgets/Slider1DWidget.h"

/// The SceneContext struct encapsulates an SG::Scene and all of the important
/// items in it that are needed by the application.
struct SceneContext {
    // The scene.
    SG::ScenePtr        scene;

    // Current frustum in window view.
    Frustum             frustum;

    // Gantry and cameras.
    SG::GantryPtr       gantry;
    SG::VRCameraPtr     vr_camera;
    SG::WindowCameraPtr window_camera;

    // Other important nodes in the scene.
    BoardPtr            floating_board;
    BoardPtr            tool_board;
    TreePanelPtr        tree_panel;
    Slider1DWidgetPtr   height_slider;
    ControllerPtr       left_controller;
    ControllerPtr       right_controller;
    InspectorPtr        inspector;
    RadialMenuPtr       left_radial_menu;
    RadialMenuPtr       right_radial_menu;
    SG::NodePtr         room;
    RootModelPtr        root_model;
    StageWidgetPtr      stage;
    SG::NodePtr         build_volume;

    /// Path to the stage, useful for converting between stage and world
    /// coordinates.
    SG::NodePath        path_to_stage;

    // Nodes and shapes that help with debugging.
    SG::TextNodePtr     debug_text;
    SG::NodePtr         debug_sphere;
    SG::LinePtr         debug_line;

    /// Fills in a SceneContext from a Scene. If is_full_scene is true, this
    /// expects everything to be filled in.
    void FillFromScene(const SG::ScenePtr &scene_in, bool is_full_scene);
};

typedef std::shared_ptr<SceneContext> SceneContextPtr;
