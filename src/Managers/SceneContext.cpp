//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Managers/SceneContext.h"

#include "Debug/Print.h"
#include "Items/BuildVolume.h"
#include "Items/Controller.h"
#include "Items/Inspector.h"
#include "Items/PrecisionControl.h"
#include "Items/RadialMenu.h"
#include "Models/RootModel.h"
#include "Panels/Board.h"
#include "Panels/KeyboardPanel.h"
#include "Panels/TreePanel.h"
#include "SG/Gantry.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/ShadowPass.h"
#include "SG/Scene.h"
#include "SG/TextNode.h"
#include "SG/VRCamera.h"
#include "SG/WindowCamera.h"
#include "Widgets/PushButtonWidget.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/StageWidget.h"

namespace {

/// Shorthand.
template <typename T>
static std::shared_ptr<T> FindTyped_(const SG::Scene &scene, const Str &name) {
    return SG::FindTypedNodeInScene<T>(scene, name);
}

/// Shorthand.
static SG::NodePtr FindNamed_(const SG::Scene &scene, const Str &name) {
    return SG::FindNodeInScene(scene, name);
}

}  // anonymous namespace

void SceneContext::FillFromScene(const SG::ScenePtr &scene_in,
                                 bool is_full_scene) {
    ASSERT(scene_in);
    scene = scene_in;

    const SG::Scene &sc = *scene;  // Shorthand.

    // Access the Gantry and cameras.
    gantry = sc.GetGantry();
    ASSERT(gantry);
    for (auto &cam: gantry->GetCameras()) {
        if (cam->GetTypeName() == "WindowCamera")
            window_camera = std::dynamic_pointer_cast<SG::WindowCamera>(cam);
        else if (cam->GetTypeName() == "VRCamera")
            vr_camera = std::dynamic_pointer_cast<SG::VRCamera>(cam);
    }
    ASSERT(window_camera);

    if (! is_full_scene)
        return;

    // Access the ShadowPass from the Scene.
    for (const auto &rp: sc.GetRenderPasses()) {
        if ((shadow_pass = std::dynamic_pointer_cast<SG::ShadowPass>(rp)))
            break;
    }
    ASSERT(shadow_pass);

    // Find all of the other important nodes.
    app_board         = FindTyped_<Board>(sc,            "AppBoard");
    key_board         = FindTyped_<Board>(sc,            "KeyBoard");
    tool_board        = FindTyped_<Board>(sc,            "ToolBoard");
    wall_board        = FindTyped_<Board>(sc,            "WallBoard");
    tree_panel        = FindTyped_<TreePanel>(sc,        "TreePanel");
    keyboard_panel    = FindTyped_<KeyboardPanel>(sc,    "KeyboardPanel");
    precision_control = FindTyped_<PrecisionControl>(sc, "PrecisionControl");
    height_pole       = FindTyped_<PushButtonWidget>(sc, "HeightPole");
    height_slider     = FindTyped_<Slider1DWidget>(sc,   "HeightSlider");
    left_controller   = FindTyped_<Controller>(sc,       "LeftController");
    right_controller  = FindTyped_<Controller>(sc,       "RightController");
    inspector         = FindTyped_<Inspector>(sc,        "Inspector");
    left_radial_menu  = FindTyped_<RadialMenu>(sc,       "LeftRadialMenu");
    right_radial_menu = FindTyped_<RadialMenu>(sc,       "RightRadialMenu");
    room              = FindNamed_(sc,                   "Room");
    work_hider        = FindNamed_(sc,                   "WorkHider");
    root_model        = FindTyped_<RootModel>(sc,        "ModelRoot");
    stage             = FindTyped_<StageWidget>(sc,      "Stage");
    build_volume      = FindTyped_<BuildVolume>(sc,      "BuildVolume");

    // The path_to_stage converts between world coordinates and stage
    // coordinates. Include the node in the StageWidget that scales based on
    // the radius.
    path_to_stage = SG::FindNodePathInScene(sc, *stage->GetStageScaleNode());

    tree_panel->SetRootModel(root_model);

#if ENABLE_DEBUG_FEATURES
    // Debugging helpers.
    debug_text     = FindTyped_<SG::TextNode>(sc, "DebugText");
    debug_rect     = FindNamed_(sc, "DebugRect");
    debug_sphere   = FindNamed_(sc, "DebugSphere");
#endif
}
