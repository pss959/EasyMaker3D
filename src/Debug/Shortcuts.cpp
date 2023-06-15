#include "Debug/Shortcuts.h"

#if ENABLE_DEBUG_FEATURES

#include <iostream>
#include <unordered_map>
#include <vector>

#include "Commands/CommandList.h"
#include "Debug/Print.h"
#include "Handlers/LogHandler.h"
#include "Items/Controller.h"
#include "Managers/SceneContext.h"
#include "Models/RootModel.h"
#include "Panels/Board.h"
#include "Panels/Panel.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/ShadowPass.h"
#include "SG/TextNode.h"
#include "Util/Assert.h"
#include "Util/KLog.h"
#include "Util/String.h"
#include "Util/Write.h"
#include "Widgets/StageWidget.h"

namespace {

// ----------------------------------------------------------------------------
// Helper classes.
// ----------------------------------------------------------------------------

/// The ShortcutMap_ class manages debugging shortcuts.
///
/// \ingroup Debug
class ShortcutMap_ {
  public:
    /// Enum representing different actions that are used to help debug
    /// applications.
    enum class Action {
        kNone,
        kDumpControllerModels,
        kPrintBounds,
        kPrintBoundsOnPath,
        kPrintCommands,
        kPrintEndNode,
        kPrintGraph,
        kPrintGraphOnPath,
        kPrintHelp,
        kPrintLocations,
        kPrintLocationsOnPath,
        kPrintMatrices,
        kPrintMatricesOnPath,
        kPrintModels,
        kPrintModelsFull,
        kPrintPaneTreeBrief,
        kPrintPaneTreeFull,
        kPrintDebugSpherePosition,
        kPrintSkeleton,
        kPrintSkeletonOnPath,
        kPrintTransforms,
        kPrintTool,
        kPrintTransformsOnPath,
        kPrintView,
        kPrintWidget,
        kReloadScene,
        kToggleEventLogging,
        kToggleLogging,
        kToggleShadows,
        kToggleSphere,
    };

    ShortcutMap_();

    /// Returns the Action corresponding to the given shortcut string, which
    /// may be kNone.
    Action GetAction(const std::string &str) const {
        return action_map_.contains(str) ? action_map_.at(str) : Action::kNone;
    }

    /// Returns the help string to print.
    const std::string & GetHelpString() const { return help_string_; }

  private:
    /// Struct storing info for each Action.
    struct ActionData_ {
        std::string shortcut;
        Action      action;
        std::string help_string;
    };

    /// Returns a vector of ActionData_ instances for all actions.
    static std::vector<ActionData_> GetData_();

    /// Maps shortcut string to an Action.
    std::unordered_map<std::string, Action> action_map_;

    /// Help string printed for the Action_::kPrintHelp shortcut.
    std::string help_string_;

    void InitActionMap_(const std::vector<ActionData_> &data);
    void InitHelpString_(const std::vector<ActionData_> &data);
};

ShortcutMap_::ShortcutMap_() {
    const std::vector<ActionData_> data = GetData_();

    // Set up the map from shortcut string to Action.
    InitActionMap_(data);

    // Set up the help string.
    InitHelpString_(data);
}

std::vector<ShortcutMap_::ActionData_> ShortcutMap_::GetData_() {
    // This vector is sorted by shortcut for the help string.
    return std::vector<ActionData_>{
        { "Alt-b",       Action::kPrintBounds,
          "Print bounds for all nodes in the scene" },
        { "Shift-Alt-b", Action::kPrintBoundsOnPath,
          "Print bounds for all nodes in the current path" },
        { "Alt-c",       Action::kPrintCommands,
          "Print the command list" },
        { "Alt-d",       Action::kPrintDebugSpherePosition,
          "Print the position of the debug sphere" },
        { "Shift-Alt-d", Action::kToggleSphere,
          "Toggle the visibility of the debug sphere" },
        { "Alt-e",       Action::kPrintEndNode,
          "Print the node graph rooted by the end of the current path" },
        { "Alt-g",       Action::kPrintGraph,
          "Print the full node graph for the scene" },
        { "Shift-Alt-g", Action::kPrintGraphOnPath,
          "Print the full node graph for current path" },
        { "Alt-h",       Action::kPrintHelp,
          "Print this help" },
        { "Alt-k",       Action::kDumpControllerModels,
          "Dump the custom controller models to files" },
        { "Alt-l",       Action::kToggleEventLogging,
          "Toggle event logging" },
        { "Shift-Alt-1", Action::kToggleLogging,
          "Toggle all logging" },
        { "Alt-m",       Action::kPrintMatrices,
          "Print matrices for all nodes in the scene" },
        { "Shift-Alt-m", Action::kPrintMatricesOnPath,
          "Print matrices for all nodes in the current path" },
        { "Alt-o",       Action::kPrintModels,
          "Print all models in the scene" },
        { "Shift-Alt-o", Action::kPrintModelsFull,
          "Print all models in the scene with model-specific fields" },
        { "Alt-p",       Action::kPrintPaneTreeBrief,
          "Print a brief pane tree for the current board" },
        { "Shift-Alt-p", Action::kPrintPaneTreeFull,
          "Print the full pane tree for the current board" },
        { "Alt-r",       Action::kReloadScene,
          "Reload the scene from resource files" },
        { "Alt-n",       Action::kPrintSkeleton,
          "Print all nodes and shapes (skeleton) in the scene" },
        { "Shift-Alt-n", Action::kPrintSkeletonOnPath,
          "Print all nodes and shapes (skeleton) on the current path" },
        { "Alt-s",       Action::kToggleShadows,
          "Toggle shadows" },
        { "Alt-t",       Action::kPrintTransforms,
          "Print transforms for all nodes in the scene" },
        { "Shift-Alt-t", Action::kPrintTransformsOnPath,
          "Print transforms for all nodes in the current path" },
        { "Alt-u",       Action::kPrintTool,
          "Print the scene subgraph for the active Tool" },
        { "Alt-v",       Action::kPrintView,
          "Print the current viewing information" },
        { "Alt-w",       Action::kPrintWidget,
          "Print the widget intersected by the debug sphere" },
        { "Alt-.",       Action::kPrintLocations,
          "Print locations for all nodes in the scene" },
        { "Shift-Alt-.", Action::kPrintLocationsOnPath,
          "Print locations for all nodes in the current path" },
    };
}

void ShortcutMap_::InitActionMap_(const std::vector<ActionData_> &data) {
    for (const auto &d: data)
        action_map_[d.shortcut] = d.action;
}

void ShortcutMap_::InitHelpString_(const std::vector<ActionData_> &data) {
    // Terminal colors.
    const std::string kColor  = "\033[36m";  // Cyan.
    const std::string kNormal = "\033[0m";

    help_string_ = "-----------------------------------------------------\n"
        "Debugging help shortcuts:\n"
        "(Current path is defined by node under mouse cursor.)\n";
    const size_t kLen = 12;
    for (const auto &d: data) {
        help_string_ += "   " + kColor + d.shortcut + kNormal +
            ":" + Util::Spaces(kLen - d.shortcut.size()) +
            d.help_string + ".\n";
    }
    help_string_ += "-----------------------------------------------------\n\n";
};

// ----------------------------------------------------------------------------
// Global state.
// ----------------------------------------------------------------------------

static ShortcutMap_    shortcut_map_;
static LogHandlerPtr   log_handler_;
static CommandListPtr  command_list_;
static SceneContextPtr scene_context_;
static SG::NodePath    limit_path_;

// ----------------------------------------------------------------------------
// Main shortcut-handling functions.
// ----------------------------------------------------------------------------

/// Returns the Board to use for handling shortcuts, depending on what is
/// visible in the scene.
const Board & GetBoard_() {
    ASSERT(scene_context_);
    const SceneContext &sc = *scene_context_;

    // These are checked in a specific order.
    if (sc.key_board->IsShown())
        return *sc.key_board;
    else if (sc.app_board->IsShown())
        return *sc.app_board;
    else if (sc.tool_board->IsShown())
        return *sc.tool_board;
    else
        return *sc.wall_board;
}

/// Returns the top Pane in the current Board.
const Pane & GetBoardPane_() {
    return *GetBoard_().GetCurrentPanel()->GetPane();
}

/// Returns a matrix to convert from world to stage coordinates.
static Matrix4f GetWorldToStageMatrix_() {
    ASSERT(scene_context_);
    ASSERT(! scene_context_->path_to_stage.empty());
    return SG::CoordConv(scene_context_->path_to_stage).GetRootToObjectMatrix();
}

/// Dumps mesh and texture image data for the given Controller to files.
static void DumpControllerModel_(const Controller &controller) {
    ModelMesh          mesh;
    ion::gfx::ImagePtr image;
    if (! controller.GetCustomModelData(mesh, image))
        return;

    const std::string name = Util::EnumToWord(controller.GetHand());

    const std::string mfn = name + "_mesh.tri";
    const std::string tfn = name + "_tex.jpg";
    if (! Util::WriteString(mfn, mesh.ToBinaryString()) ||
        ! Util::WriteImage(tfn, *image, true)) {
        std::cerr << "*** Unable to dump" << name
                  << " mesh and texture image to " << mfn << " / "
                  << tfn << "\n";
    }
    else {
        std::cerr << "=== Dumped " << name << " mesh and texture image to "
                  << mfn << " / " << tfn << "\n";
    }
}

/// Handles the ShortcutMap_::Action::kPrintWidget case.
static void PrintTouchedWidget_() {
    const auto &board = GetBoard_();
    const float rad = scene_context_->left_controller->GetTouchRadius();
    Point3f pos = Point3f(scene_context_->debug_sphere->GetTranslation());
    pos[2] += .25f * rad;
    const auto widget = board.GetTouchedWidget(pos, rad);
    if (widget) {
        const auto path = SG::FindNodePathUnderNode(
            board.GetCurrentPanel()->GetPane(), *widget);
        std::cout << "Widget path = " << path.ToString(true) << "\n";
    }
    else {
        std::cout << "NO WIDGET\n";
    }
}

/// Does all of the work for HandleShortcut().
static bool HandleShortcut_(const std::string &str) {
    ASSERT(scene_context_);
    ASSERT(scene_context_->scene);
    const SG::Node &root = *scene_context_->scene->GetRootNode();

    const auto action = shortcut_map_.GetAction(str);
    switch (action) {
        using enum ShortcutMap_::Action;
      case kNone:
        break;
      case kDumpControllerModels:
        DumpControllerModel_(*scene_context_->left_controller);
        DumpControllerModel_(*scene_context_->right_controller);
        break;
      case kPrintBounds:
        Debug::PrintBounds(root, GetWorldToStageMatrix_());
        break;
      case kPrintBoundsOnPath:
        Debug::PrintBoundsOnPath(limit_path_, GetWorldToStageMatrix_());
        break;
      case kPrintCommands:
        ASSERT(command_list_);
        Debug::PrintCommands(*command_list_);
        break;
      case kPrintDebugSpherePosition:
        std::cout << "Sphere at "
                  << scene_context_->debug_sphere->GetTranslation() << "\n";
        break;
      case kPrintEndNode:
        if (! limit_path_.empty())
            Debug::PrintGraph(*limit_path_.back());
        break;
      case kPrintGraph:
        Debug::PrintGraph(root);
        break;
      case kPrintGraphOnPath:
        Debug::PrintGraphOnPath(limit_path_);
        break;
      case kPrintHelp:
        std::cout << shortcut_map_.GetHelpString();
        break;
      case kPrintLocations:
        Debug::PrintLocations(root, GetWorldToStageMatrix_());
        break;
      case kPrintLocationsOnPath:
        Debug::PrintLocationsOnPath(limit_path_, GetWorldToStageMatrix_());
        break;
      case kPrintMatrices:
        Debug::PrintMatrices(root);
        break;
      case kPrintMatricesOnPath:
        Debug::PrintMatricesOnPath(limit_path_);
        break;
      case kPrintModels:
        Debug::PrintModels(*scene_context_->root_model, false);
        break;
      case kPrintModelsFull:
        Debug::PrintModels(*scene_context_->root_model, true);
        break;
      case kPrintPaneTreeBrief:
        Debug::PrintPaneTree(GetBoardPane_(), true);
        break;
      case kPrintPaneTreeFull:
        Debug::PrintPaneTree(GetBoardPane_(), false);
        break;
      case kPrintSkeleton:
        Debug::PrintNodesAndShapes(root);
        break;
      case kPrintSkeletonOnPath:
        Debug::PrintNodesAndShapesOnPath(limit_path_);
        break;
      case kPrintTransforms:
        Debug::PrintTransforms(root);
        break;
      case kPrintTransformsOnPath:
        Debug::PrintTransformsOnPath(limit_path_);
        break;
      case kPrintTool:
        Debug::PrintTransformsOnPath(
            SG::FindNodePathInScene(*scene_context_->scene, "ToolParent"),
            true);
        break;
      case kPrintView:
        Debug::PrintViewInfo(*scene_context_->frustum, *scene_context_->stage);
        break;
      case kPrintWidget:
        PrintTouchedWidget_();
        break;
      case kReloadScene:
        ASSERTM(false, "Should have handled reload-scene shortcut elsewhere");
        break;
      case kToggleEventLogging:
        if (log_handler_)
            log_handler_->SetEnabled(! log_handler_->IsEnabled());
        break;
      case kToggleLogging:
        KLogger::ToggleLogging();
        break;
      case kToggleShadows: {
          auto &sp = *scene_context_->shadow_pass;
          sp.SetShadowsEnabled(! sp.AreShadowsEnabled());
          break;
      }
      case kToggleSphere: {
          auto &ds = *scene_context_->debug_sphere;
          const auto flag = SG::Node::Flag::kRender;
          ds.SetFlagEnabled(flag, ! ds.IsFlagEnabled(flag));
          break;
      }
    }

    return action != ShortcutMap_::Action::kNone;
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

namespace Debug {

void SetLogHandler(const LogHandlerPtr &log_handler) {
    ASSERT(log_handler);
    log_handler_ = log_handler;
}

void SetCommandList(const CommandListPtr &command_list) {
    ASSERT(command_list);
    command_list_ = command_list;
}

void SetSceneContext(const SceneContextPtr &scene_context) {
    ASSERT(scene_context);
    scene_context_ = scene_context;
}

void SetLimitPath(const SG::NodePath &path) {
    limit_path_ = path;
}

void ShutDown() {
    command_list_.reset();
    scene_context_.reset();
    limit_path_.clear();
}

bool HandleShortcut(const std::string &str) {
    return HandleShortcut_(str);
}

void DisplayDebugText(const std::string &text) {
    ASSERT(scene_context_);
    const auto &dt = scene_context_->debug_text;
    ASSERT(dt);
    if (text.empty()) {
        dt->SetEnabled(false);
    }
    else {
        dt->SetText(text);
        dt->SetEnabled(true);
    }
}

void DisplayDebugSphereStage(const Point3f &pos, float diameter) {
    // Convert from stage to world coordinates.
    DisplayDebugSphereWorld(
        SG::CoordConv(scene_context_->path_to_stage).ObjectToRoot(pos),
        diameter);
}

void DisplayDebugSphereWorld(const Point3f &pos, float diameter) {
    auto &ds = *scene_context_->debug_sphere;
    ds.SetEnabled(true);
    ds.SetFlagEnabled(SG::Node::Flag::kRender, true);
    ds.SetUniformScale(diameter);
    ds.SetTranslation(pos);
}

}  // namespace Debug

#endif
