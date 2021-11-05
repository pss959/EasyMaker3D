#include "Managers/ActionManager.h"

#include <unordered_set>

#include "Assert.h"
#include "Commands/CreatePrimitiveModelCommand.h"
#include "Enums/PrimitiveType.h"
#include "Items/Board.h"
#include "Panels/Panel.h"
#include "Panes/ContainerPane.h"
#include "Parser/Writer.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/Shape.h"
#include "Util/Enum.h"

// ----------------------------------------------------------------------------
// ActionManager::Impl_ class.
// ----------------------------------------------------------------------------

class ActionManager::Impl_ {
  public:
    Impl_(const ContextPtr &context);

    void SetReloadFunc(const ReloadFunc &func) { reload_func_ = func; }
    std::string GetHelpTooltip(Action action);
    std::string GetRegularTooltip(Action action);
    bool CanApplyAction(Action action) const;
    void ApplyAction(Action action);
    bool ShouldQuit() const { return should_quit_; }

  private:
    ContextPtr            context_;
    bool                  should_quit_ = false;
    std::function<void()> reload_func_;

    /// Convenience that returns the current Selection.
    const Selection & GetSelection() const {
        return context_->selection_manager->GetSelection();
    }

    /// Adds a command to create a primitive model of the given type.
    void CreatePrimitiveModel_(PrimitiveType type);

    /// Convenience to get the current scene.
    SG::Scene & GetScene() const { return *context_->scene_context->scene; }

#if DEBUG
    /// \name Debugging functions.
    /// Each of these handles an Action (in debug builds only) that help debug
    /// the application.
    ///@{
    void PrintBounds_();
    void PrintMatrices_();
    void PrintNodesAndShapes_();
    void PrintPanes_();
    void PrintScene_();
    void ReloadScene_();

    static void PrintNodeBounds_(const SG::Node &node, int level,
                                 const Matrix4f &start_matrix);
    static void PrintNodeMatrices_(const SG::Node &node, int level,
                                   const Matrix4f &start_matrix);
    static void PrintNodesAndShapes_(
        const SG::Node &node, int level,
        std::unordered_set<const SG::Object *> &done);

    static void PrintPaneTree_(const Pane &pane, int level);

    static std::string Indent_(int level, bool add_horiz = true) {
        std::string s;
        for (int i = 1; i < level; ++i)
            s += "| ";
        if (level >= 1)
            s += add_horiz ? "|-" : "| ";
        return s;
    }
    ///@}
#endif
};

ActionManager::Impl_::Impl_(const ContextPtr &context) : context_(context) {
    ASSERT(context);
    ASSERT(context->tool_context);
    ASSERT(context->command_manager);
    ASSERT(context->selection_manager);
    ASSERT(context->target_manager);
    ASSERT(context->tool_manager);
    ASSERT(context->main_handler);
}

std::string ActionManager::Impl_::GetHelpTooltip(Action action) {
    // Only certain actions have a different version for help. All toggles do,
    // since the help string does not indicate the state.
    std::string s;

    switch (action) {
      case Action::kUndo:
        s = "Undo the last command";
        break;
      case Action::kRedo:
        s = "Redo the last undone command";
        break;

      case Action::kToggleSpecializedTool:
        s = "Switch between the current general tool and the"
            " specialized tool for the selected models";
        break;

      case Action::kTogglePointTarget:
        s = "Activate or deactivate the point target";
        break;
      case Action::kToggleEdgeTarget:
        s = "Activate or deactivate the edge target";
        break;

      case Action::kRadialLayout:
        s = "Lay out selected models along a circular arc";
        break;

      case Action::kToggleAxisAligned:
        s = "Transform models in local or global coordinates";
        break;

      case Action::kInspectSelection:
        s = "Inspect the current primary selection";
        break;
      case Action::kToggleBuildVolume:
        s = "Show or hide the translucent build volume";
        break;
      case Action::kToggleShowEdges:
        s = "Show or hide edges on all models";
        break;

      case Action::kToggleLeftRadialMenu:
        s = "Show or hide the left radial menu";
        break;
      case Action::kToggleRightRadialMenu:
        s = "Show or hide the right radial menu";
        break;

      default:
        // All others return the regular tooltip.
        s = GetRegularTooltip(action);
    }

    if (Util::StringContains(Util::EnumName(action), "kToggle"))
        s = "TOGGLE: " + s;
    return s;
}

std::string ActionManager::Impl_::GetRegularTooltip(Action action) {
    // Helper string when needed.
    std::string a;

    switch (action) {
      case Action::kUndo: {
        auto &cl = context_->command_manager->GetCommandList();
        return "Undo the last command <" +
            cl.GetCommandToUndo()->GetDescription() + ">";
      }
      case Action::kRedo: {
        auto &cl = context_->command_manager->GetCommandList();
        return "Redo the last undone command <" +
            cl.GetCommandToRedo()->GetDescription() + ">";
      }

      case Action::kQuit:
        return "Exit the application";

      case Action::kOpenSessionPanel:
        return "Open the panel to save or open session files";
      case Action::kOpenSettingsPanel:
        return "Edit application settings";
      case Action::kOpenInfoPanel:
        return "Open the panel to show information about selected models";
      case Action::kOpenHelpPanel:
        return "Open the panel to access help";

      case Action::kCreateBox:
        return "Create a primitive Box model";
      case Action::kCreateCylinder:
        return "Create a primitive Cylinder model";
      case Action::kCreateImportedModel:
        return "Import a model from a file";
      case Action::kCreateRevSurf:
        return "Create a model that is a surface of revolution";
      case Action::kCreateSphere:
        return "Create a primitive Sphere model";
      case Action::kCreateText:
        return "Create a 3D Text model";
      case Action::kCreateTorus:
        return "Create a primitive Torus model";

      case Action::kConvertBevel:
        return "Convert selected models to beveled models";
      case Action::kConvertClip:
        return "Convert selected models to clipped models";
      case Action::kConvertMirror:
        return "Convert selected models to mirrored models";

      case Action::kCombineCSGDifference:
        return "Create a CSG Difference from selected objects";
      case Action::kCombineCSGIntersection:
        return "Create a CSG Intersection from selected objects";
      case Action::kCombineCSGUnion:
        return "Create a CSG Union from selected objects";
      case Action::kCombineHull:
        return "Create a model that is the convex hull of selected models";

      case Action::kColorTool:
        return "Edit the color of the selected models";
      case Action::kComplexityTool:
        return "Edit the complexity of the selected models";
      case Action::kRotationTool:
        return "Rotate the selected models (Alt for in-place)";
      case Action::kScaleTool:
        return "Change the size of the selected models (Alt for symmetric)";
      case Action::kTranslationTool:
        return "Change the position of the selected models";

      case Action::kSwitchToPreviousTool:
        return "Switch to the previous general tool";
      case Action::kSwitchToNextTool:
        return "Switch to the next general tool";

      case Action::kToggleSpecializedTool:
        if (context_->tool_manager->IsUsingSpecializedTool()) {
            return "Switch back to the current general tool";
        }
        else {
            auto tool = context_->tool_manager->GetSpecializedToolForSelection(
                GetSelection());
            return "Switch to the specialized " + tool->GetTypeName();
        }

      case Action::kDecreaseComplexity:
        return "Decrease the complexity of the selected models by .05";
      case Action::kIncreaseComplexity:
        return "Increase the complexity of the selected models by .05";

      case Action::kDecreasePrecision:
        return "Decrease the current precision";
      case Action::kIncreasePrecision:
        return "Increase the current precision";

      case Action::kMoveToOrigin:
        return "Move the primary selection to the origin";

      case Action::kSelectAll:
        return "Select all top-level models";
      case Action::kSelectNone:
        return "Deselect all selected models";
      case Action::kSelectParent:
        return "Select the parent of the primary selection";
      case Action::kSelectFirstChild:
        return "Select the first child of the primary selection";
      case Action::kSelectPreviousSibling:
        return "Select the previous sibling of the primary selection";
      case Action::kSelectNextSibling:
        return "Select the next sibling of the primary selection";

      case Action::kDelete:
        return "Delete all selected models";
      case Action::kCut:
        return "Cut all selected models to the clipboard";
      case Action::kCopy:
        return "Copy all selected models to the clipboard";
      case Action::kPaste:
        return "Paste all models from the clipboard";
      case Action::kPasteInto:
        return "Paste all models from the clipboard as children of"
            " the selected model";

#if XXXX
      case Action::kTogglePointTarget:
        a = context_->target_manager->IsPointTargetVisible() ?
            "Deactivate" : "Activate";
        return a + " the point target";
      case Action::kToggleEdgeTarget:
        a = context_->target_manager->IsEdgeTargetVisible() ?
            "Deactivate" : "Activate";
        return a + " the edge target";
#else
      case Action::kTogglePointTarget: return "XXXX kTogglePointTarget";
      case Action::kToggleEdgeTarget:  return "XXXX kToggleEdgeTarget";
#endif

      case Action::kLinearLayout:
        return "Lay out the centers of the selected models along a"
            " line using the edge target";
      case Action::kRadialLayout:
        if (GetSelection().GetCount() == 1U)
            return "Move the bottom center of the selected Model to the"
                " point target and orient its +Y axis to the point"
                "  target direction";
        else
            return "Lay out the bottom centers of the selected models"
                " along a circular arc using the point target radial layout";

      case Action::kToggleAxisAligned:
        return context_->tool_context->is_axis_aligned ?
            "Transform models in their local coordinates" :
            "Transform models relative to global coordinate axes";

      case Action::kMovePrevious:
        return "Move the selected model up in the order";
      case Action::kMoveNext:
        return "Move the selected model down in the order";

      case Action::kInspectSelection:
        return "Inspect the current primary selection";
      case Action::kToggleBuildVolume:
        // XXXX a = _context.buildVolumeGO.activeSelf ? "Hide" : "Show";
        return "Hide/Show the build volume";
      case Action::kToggleShowEdges:
        // XXXX a = _context.modelManager.AreEdgesShown() ? "Hide" : "Show";
        return "Hide/Show edges on all models";

      case Action::kHideSelected:
        return "Hide selected top-level models";
      case Action::kShowAll:
        return "Show all hidden top-level models";

      case Action::kToggleLeftRadialMenu:
        // XXXX a = _context.leftRadialMenu.IsActive() ? "Hide" : "Show";
        return "Hide/Show the left radial menu";
      case Action::kToggleRightRadialMenu:
        // XXXX a = _context.rightRadialMenu.IsActive() ? "Hide" : "Show";
        return "Hide/Show the right radial menu";

      case Action::kEditName:
        return "Edit the name of the primary selection";

      default:
        // Only the None action is allowed to not have a tooltip.
        ASSERTM(action == Action::kNone,
                "No tooltip for AppAction " + Util::EnumName(action));
        return "";
    }
}

bool ActionManager::Impl_::CanApplyAction(Action action) const {
    // XXXX Need to flesh this out...
    switch (action) {

      case Action::kUndo:
        return context_->command_manager->CanUndo();
      case Action::kRedo:
        return context_->command_manager->CanRedo();

      default:
        // Anything else is assumed to always be possible.
        return true;
    }
}

void ActionManager::Impl_::ApplyAction(Action action) {
    ASSERT(CanApplyAction(action));

    // XXXX Need to flesh this out...
    switch (action) {
      case Action::kUndo:
        context_->command_manager->Undo();
        break;
      case Action::kRedo:
        context_->command_manager->Redo();
        break;
      case Action::kQuit:
        should_quit_ = true;
        break;

      case Action::kCreateBox:
        CreatePrimitiveModel_(PrimitiveType::kBox);
        break;
      case Action::kCreateCylinder:
        CreatePrimitiveModel_(PrimitiveType::kCylinder);
        break;
      case Action::kCreateSphere:
        CreatePrimitiveModel_(PrimitiveType::kSphere);
        break;
      case Action::kCreateTorus:
        CreatePrimitiveModel_(PrimitiveType::kTorus);
        break;

#if defined DEBUG
      case Action::kPrintBounds:         PrintBounds_();         break;
      case Action::kPrintMatrices:       PrintMatrices_();       break;
      case Action::kPrintNodesAndShapes: PrintNodesAndShapes_(); break;
      case Action::kPrintPanes:          PrintPanes_();          break;
      case Action::kPrintScene:          PrintScene_();          break;
      case Action::kReloadScene:         ReloadScene_();         break;
#endif

      default:
        // XXXX Do something for real.
        std::cerr << "XXXX Unimplemented action "
                  << Util::EnumName(action) << "\n";
    }
}

void ActionManager::Impl_::CreatePrimitiveModel_(PrimitiveType type) {
    CreatePrimitiveModelCommandPtr cpc =
        Parser::Registry::CreateObject<CreatePrimitiveModelCommand>();
    cpc->SetType(type);
    context_->command_manager->AddAndDo(cpc);
    context_->tool_manager->UseSpecializedTool(GetSelection());
}

void ActionManager::Impl_::PrintBounds_() {
    std::cout << "--------------------------------------------------\n";
    PrintNodeBounds_(*GetScene().GetRootNode(), 0, Matrix4f::Identity());
    std::cout << "--------------------------------------------------\n";
}

void ActionManager::Impl_::PrintMatrices_() {
    std::cout << "--------------------------------------------------\n";
    PrintNodeMatrices_(*GetScene().GetRootNode(), 0, Matrix4f::Identity());
    std::cout << "--------------------------------------------------\n";
}

void ActionManager::Impl_::PrintNodesAndShapes_() {
    std::cout << "--------------------------------------------------\n";
    std::unordered_set<const SG::Object *> done;
    PrintNodesAndShapes_(*GetScene().GetRootNode(), 0, done);
    std::cout << "--------------------------------------------------\n";
}

void ActionManager::Impl_::PrintPanes_() {
    std::cout << "--------------------------------------------------\n";
    auto board = SG::FindTypedNodeInScene<Board>(GetScene(), "FloatingBoard");
    std::cout << board->GetDesc()
              << " size = " << board->GetSize()
              << " pos = " << board->GetTranslation() << "\n";
    std::cout << "\nPANES:\n";
    PrintPaneTree_(*board->GetPanel()->GetPane(), 0);
    std::cout << "--------------------------------------------------\n";
}

void ActionManager::Impl_::PrintScene_() {
    std::cout << "--------------------------------------------------\n";
    Parser::Writer writer;
    writer.SetAddressFlag(true);
    writer.WriteObject(GetScene(), std::cout);
    std::cout << "--------------------------------------------------\n";
}

void ActionManager::Impl_::ReloadScene_() {
    ASSERT(reload_func_);
    reload_func_();
}

void ActionManager::Impl_::PrintNodeBounds_(const SG::Node &node, int level,
                                            const Matrix4f &start_matrix) {
    std::string indent = Indent_(level);
    const Matrix4f ctm = start_matrix * node.GetModelMatrix();

    auto print_bounds = [indent, ctm](const SG::Object &obj,
                                      const Bounds &bounds){
        const Bounds wbounds = TransformBounds(bounds, ctm);
        std::cout << indent << obj.GetDesc() << "\n"
                  << indent << "    LOC: " <<  bounds.ToString() << "\n"
                  << indent << "    WLD: " << wbounds.ToString() << "\n";
    };

    print_bounds(node, node.GetBounds());
    indent = Indent_(level + 1);

    for (const auto &shape: node.GetShapes())
        print_bounds(*shape, shape->GetBounds());

    for (const auto &child: node.GetChildren())
        PrintNodeBounds_(*child, level + 1, ctm);
}

void ActionManager::Impl_::PrintNodeMatrices_(const SG::Node &node, int level,
                                              const Matrix4f &start_matrix) {
    std::cout << Indent_(level) << node.GetDesc() << " "
              << node.GetModelMatrix() << "\n";

    const Matrix4f combined = start_matrix * node.GetModelMatrix();
    std::cout << Indent_(level, false) << "        => " << combined << "\n";

    for (const auto &child: node.GetChildren())
        PrintNodeMatrices_(*child, level + 1, combined);
}

void ActionManager::Impl_::PrintNodesAndShapes_(
    const SG::Node &node, int level,
    std::unordered_set<const SG::Object *> &done) {
    std::cout << Indent_(level) << node.GetDesc();
    if (! node.IsEnabled(SG::Node::Flag::kTraversal))
        std::cout << " (DISABLED)";
    if (done.find(&node) != done.end()) {
        std::cout << ";\n";
    }
    else {
        done.insert(&node);
        std::cout << "\n";
        for (const auto &shape: node.GetShapes()) {
            std::cout << Indent_(level + 1) << shape->GetDesc();
            if (done.find(shape.get()) != done.end()) {
                std::cout << ";\n";
            }
            else {
                done.insert(shape.get());
                std::cout << "\n";
            }
        }
        for (const auto &child: node.GetChildren())
            PrintNodesAndShapes_(*child, level + 1, done);
    }
}

void ActionManager::Impl_::PrintPaneTree_(const Pane &pane, int level) {
    std::cout << Indent_(level) << pane.GetDesc()
              << "S="   << Util::ToString(pane.GetSize(), .01f)
              << " MS=" << Util::ToString(pane.GetMinSize(), .01f)
              << " R=[" << (pane.IsWidthResizable()  ? 'T' : 'F')
              << ","    << (pane.IsHeightResizable() ? 'T' : 'F')
             << "]\n";

    if (const ContainerPane *cp = dynamic_cast<const ContainerPane *>(&pane))
        for (const auto &subpane: cp->GetPanes())
            PrintPaneTree_(*subpane, level + 1);
}

// ----------------------------------------------------------------------------
// ActionManager functions.
// ----------------------------------------------------------------------------

ActionManager::ActionManager(const ContextPtr &context) :
    impl_(new Impl_(context)) {
}

ActionManager::~ActionManager() {
}

void ActionManager::SetReloadFunc(const ReloadFunc &func) {
    impl_->SetReloadFunc(func);
}

std::string ActionManager::GetActionTooltip(Action action, bool for_help) {
    return for_help ? impl_->GetHelpTooltip(action) :
        impl_->GetRegularTooltip(action);
}

bool ActionManager::CanApplyAction(Action action) const {
    return impl_->CanApplyAction(action);
}

void ActionManager::ApplyAction(Action action) {
    impl_->ApplyAction(action);
}

bool ActionManager::ShouldQuit() const {
    return impl_->ShouldQuit();
}
