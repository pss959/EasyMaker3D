//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"
#include "SG/NodePath.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(SceneContext);
DECL_SHARED_PTR(Board);
DECL_SHARED_PTR(BuildVolume);
DECL_SHARED_PTR(Controller);
DECL_SHARED_PTR(Frustum);
DECL_SHARED_PTR(Inspector);
DECL_SHARED_PTR(KeyboardPanel);
DECL_SHARED_PTR(PrecisionControl);
DECL_SHARED_PTR(PushButtonWidget);
DECL_SHARED_PTR(RadialMenu);
DECL_SHARED_PTR(RootModel);
DECL_SHARED_PTR(TreePanel);
DECL_SHARED_PTR(StageWidget);
DECL_SHARED_PTR(Slider1DWidget);

namespace SG {
DECL_SHARED_PTR(Gantry);
DECL_SHARED_PTR(Node);
DECL_SHARED_PTR(Scene);
DECL_SHARED_PTR(ShadowPass);
DECL_SHARED_PTR(TextNode);
DECL_SHARED_PTR(VRCamera);
DECL_SHARED_PTR(WindowCamera);
}

/// The SceneContext struct encapsulates an SG::Scene and all of the important
/// items in it that are needed by the application.
///
/// \ingroup Managers
struct SceneContext {
    // The scene.
    SG::ScenePtr        scene;

    // Current frustum in window view.
    FrustumPtr          frustum;

    // Gantry and cameras.
    SG::GantryPtr       gantry;
    SG::VRCameraPtr     vr_camera;
    SG::WindowCameraPtr window_camera;

    // Other important nodes in the scene.
    SG::ShadowPassPtr   shadow_pass;
    BoardPtr            app_board;
    BoardPtr            key_board;
    BoardPtr            tool_board;
    BoardPtr            wall_board;
    TreePanelPtr        tree_panel;
    KeyboardPanelPtr    keyboard_panel;
    PrecisionControlPtr precision_control;
    PushButtonWidgetPtr height_pole;
    Slider1DWidgetPtr   height_slider;
    ControllerPtr       left_controller;
    ControllerPtr       right_controller;
    InspectorPtr        inspector;
    RadialMenuPtr       left_radial_menu;
    RadialMenuPtr       right_radial_menu;
    SG::NodePtr         room;
    SG::NodePtr         work_hider;
    RootModelPtr        root_model;
    StageWidgetPtr      stage;
    BuildVolumePtr      build_volume;

    /// Path to the stage, useful for converting between stage and world
    /// coordinates.
    SG::NodePath        path_to_stage;

    // Nodes and shapes that help with debugging.
    SG::TextNodePtr     debug_text;
    SG::NodePtr         debug_rect;
    SG::NodePtr         debug_sphere;

    /// Fills in a SceneContext from a Scene. If is_full_scene is true, this
    /// expects everything to be filled in.
    void FillFromScene(const SG::ScenePtr &scene_in, bool is_full_scene);
};
