#include "App/SceneContext.h"

#include "Debug/Print.h"
#include "Items/Board.h"
#include "Items/Controller.h"
#include "Items/Inspector.h"
#include "Items/RadialMenu.h"
#include "Models/RootModel.h"
#include "Panels/TreePanel.h"
#include "SG/Gantry.h"
#include "SG/Line.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/TextNode.h"
#include "SG/VRCamera.h"
#include "SG/WindowCamera.h"
#include "Util/General.h"
#include "Widgets/Slider1DWidget.h"
#include "Widgets/StageWidget.h"

namespace {

/// Shorthand.
template <typename T>
static std::shared_ptr<T> FindTyped_(const SG::Scene &scene,
                                     const std::string &name) {
    return SG::FindTypedNodeInScene<T>(scene, name);
}

/// Shorthand.
static SG::NodePtr FindNamed_(const SG::Scene &scene, const std::string &name) {
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
            window_camera = Util::CastToDerived<SG::WindowCamera>(cam);
        else if (cam->GetTypeName() == "VRCamera")
            vr_camera = Util::CastToDerived<SG::VRCamera>(cam);
    }
    ASSERT(window_camera);

    if (! is_full_scene)
        return;

    // Find all of the other important nodes.
    floating_board    = FindTyped_<Board>(sc,          "FloatingBoard");
    tool_board        = FindTyped_<Board>(sc,          "ToolBoard");
    tree_panel        = FindTyped_<TreePanel>(sc,      "TreePanel");
    height_slider     = FindTyped_<Slider1DWidget>(sc, "HeightSlider");
    left_controller   = FindTyped_<Controller>(sc,     "LeftController");
    right_controller  = FindTyped_<Controller>(sc,     "RightController");
    inspector         = FindTyped_<Inspector>(sc,      "Inspector");
    left_radial_menu  = FindTyped_<RadialMenu>(sc,     "LeftRadialMenu");
    right_radial_menu = FindTyped_<RadialMenu>(sc,     "RightRadialMenu");
    room              = FindNamed_(sc,                 "Room");
    model_hider       = FindNamed_(sc,                 "ModelHider");
    root_model        = FindTyped_<RootModel>(sc,      "ModelRoot");
    stage             = FindTyped_<StageWidget>(sc,    "Stage");
    build_volume      = FindNamed_(sc,                 "BuildVolume");

    path_to_stage = SG::FindNodePathInScene(sc, *stage);

    tree_panel->SetRootModel(root_model);

#if DEBUG
    // Debugging helpers.
    debug_sphere     = FindNamed_(sc, "DebugSphere");
    debug_text       = FindTyped_<SG::TextNode>(sc, "DebugText");
    auto line_node   = FindNamed_(sc, "DebugLine");
    debug_line = SG::FindTypedShapeInNode<SG::Line>(*line_node, "Line");
#endif
}
