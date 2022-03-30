#include "Managers/ActionManager.h"

#include <unordered_set>

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Commands/ChangeComplexityCommand.h"
#include "Commands/ChangeOrderCommand.h"
#include "Commands/CommandList.h"
#include "Commands/ConvertBevelCommand.h"
#include "Commands/CreateCSGModelCommand.h"
#include "Commands/CreateHullModelCommand.h"
#include "Commands/CreateImportedModelCommand.h"
#include "Commands/CreatePrimitiveModelCommand.h"
#include "Commands/CreateTextModelCommand.h"
#include "Commands/TranslateCommand.h"
#include "Enums/Hand.h"
#include "Enums/PrimitiveType.h"
#include "Handlers/MainHandler.h"
#include "Items/Board.h"
#include "Items/Inspector.h"
#include "Items/RadialMenu.h"
#include "Managers/ClipboardManager.h"
#include "Managers/CommandManager.h"
#include "Managers/NameManager.h"
#include "Managers/PanelManager.h"
#include "Managers/PrecisionManager.h"
#include "Managers/SelectionManager.h"
#include "Managers/SettingsManager.h"
#include "Managers/TargetManager.h"
#include "Managers/ToolManager.h"
#include "Models/BeveledModel.h"
//#include "Models/ClippedModel.h"
#include "Models/CombinedModel.h"
#include "Models/HullModel.h"
//#include "Models/MirroredModel.h"
#include "Panels/InfoPanel.h"
#include "Panels/Panel.h"
#include "Panels/TreePanel.h"
#include "Parser/Registry.h"
#include "SceneContext.h"
#include "SessionState.h"
#include "Settings.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SelPath.h"
#include "Selection.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/General.h"

// ----------------------------------------------------------------------------
// Convenient helper functions.
// ----------------------------------------------------------------------------

namespace {

/// Returns true if all Models in the given Selection have valid meshes.
static bool AreAllMeshesValid_(const Selection &sel) {
    std::string reason;
    for (const auto &sel_path: sel.GetPaths())
        if (! sel_path.GetModel()->IsMeshValid(reason))
            return false;
    return true;
}

/// Returns true if all Models in the given Selection are at the top level.
static bool AreAllTopLevel_(const Selection &sel) {
    for (const auto &sel_path: sel.GetPaths())
        if (! sel_path.GetModel()->IsTopLevel())
            return false;
    return true;
}

/// Returns true if at least one Model in the given Selection is not of the
/// templated type and can therefore be converted to it.
template <typename T> static bool CanConvert_(const Selection &sel) {
    for (const auto &sel_path: sel.GetPaths())
        if (! Util::CastToDerived<T>(sel_path.GetModel()))
            return true;
    return false;
}

/// Creates a Command of the templated type.
template <typename T> static std::shared_ptr<T> CreateCommand_() {
    return Parser::Registry::CreateObject<T>();
}

/// Returns true if there are Models in the given Selection that can be deleted.
static bool CanDeleteModels_(const Selection &sel) {
    if (! sel.HasAny())
        return false;

    std::vector<ModelPtr> models_in_path;
    for (const auto &sel_path: sel.GetPaths()) {
        // A Model can be deleted if it is at the top level or if it is a child
        // of a CombinedModel that will allow it to be deleted.
        auto model = sel_path.GetModel();
        if (model->IsTopLevel())
            continue;

        // The path should contain at least the RootModel (which we ignore),
        // the Model's parent, and the Model.
        ASSERT(sel_path.size() >= 3U);
        auto parent = sel_path[sel_path.size() - 2];
        auto combined_model = Util::CastToDerived<CombinedModel>(parent);
        if (! combined_model)
            return false;

        // Simple rejection test: if one child was deleted, would it violate
        // the minimum count?
        const size_t child_count = combined_model->GetChildModelCount();
        ASSERT(child_count > 0);
        if (child_count - 1 < combined_model->GetMinChildCount())
            return false;

        // More accurate test: count selected children and see if subtracting
        // them would violate the minimum count.
        size_t num_selected = 0;
        for (size_t i = 0; i < child_count; ++i) {
            if (combined_model->GetChildModel(i)->IsSelected())
                ++num_selected;
        }
        if (child_count - num_selected < combined_model->GetMinChildCount())
            return false;
    }
    return true;
}

/// Returns 2 flags indicating whether the primary selection can be moved in
/// order to the previous or next position.
static void GetMoveFlags_(const Selection &sel,
                          bool &can_move_prev, bool &can_move_next) {
    can_move_prev = can_move_next = false;

    // Has to be a single Model selected.
    if (sel.GetCount() != 1U)
        return;

    // There must be at least the RootModel and the selected Model in the path.
    const auto &sel_path = sel.GetPrimary();
    ASSERT(sel_path.size() >= 2U);

    // Get the index of the selected Model within its parent.
    const auto model  = sel_path.GetModel();
    const auto parent = sel_path.GetParentModel();

    // The parent has to be a CombinedModel or the RootModel.
    if (model->IsTopLevel() || Util::IsA<CombinedModel>(parent)) {
        const int index = parent->GetChildModelIndex(model);
        ASSERT(index >= 0);
        can_move_prev = index > 0;
        can_move_next =
            static_cast<size_t>(index + 1) < parent->GetChildModelCount();
    }
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// ActionManager::Impl_ class.
// ----------------------------------------------------------------------------

class ActionManager::Impl_ {
  public:
    Impl_(const ContextPtr &context);

    void Reset();
    void UpdateFromSessionState(const SessionState &state);
    void SetReloadFunc(const ReloadFunc &func) { reload_func_ = func; }
    void ProcessUpdate();
    std::string GetHelpTooltip(Action action);
    std::string GetRegularTooltip(Action action);
    bool CanApplyAction(Action action) const;
    void ApplyAction(Action action);
    bool GetToggleState(Action action) const { return GetToggleState_(action); }
    bool ShouldQuit() const { return should_quit_; }

  private:
    ContextPtr            context_;
    ClipboardManager      clipboard_manager_;
    bool                  should_quit_ = false;
    std::function<void()> reload_func_;

    /// Tooltip string for each Action that does not change by context.
    std::vector<std::string> tooltip_strings_;

    /// Flag for each Action that indicates whether it can be applied.
    std::vector<bool>        is_action_enabled_;

    /// Returns the current state for a toggle Action.
    bool GetToggleState_(Action action) const;

    /// Changes the current state for a toggle Action.
    void SetToggleState_(Action action, bool state);

    /// Sets all tooltip strings that do not change by context.
    void SetConstantTooltipStrings_();

    /// Returns a tooltip string for an Action that requires context to set up.
    std::string GetUpdatedTooltip_(Action action);

    /// Updates all is_action_enabled_ flags that can vary.
    void UpdateEnabledFlags_();

    /// Convenience that returns the current Selection.
    const Selection & GetSelection() const {
        return context_->selection_manager->GetSelection();
    }

    /// Opens and sets up the InfoPanel.
    void OpenInfoPanel_();

    /// Adds a Command to create a CSGModel with the given operation.
    void CreateCSGModel_(CSGOperation op);

    /// Adds a Command to create a HullModel.
    void CreateHullModel_();

    /// Adds a Command to create an ImportedModel.
    void CreateImportedModel_();

    /// Adds a Command to create a PrimitiveModel of the given type.
    void CreatePrimitiveModel_(PrimitiveType type);

    /// Adds a Command to create a TextModel.
    void CreateTextModel_();

    /// Adds the given ConvertCommand to convert the current selected Models.
    void ConvertModels_(const ConvertCommandPtr &command);

    /// Modifies the complexity of all selected models by the given amount.
    void ChangeComplexity_(float delta);

    /// Moves the bottom center of the primary Model is at the origin and all
    /// other selected Models by the same amount.
    void MoveSelectionToOrigin_();

    /// Moves the primary selection earlier or later in the list of top-level
    /// Models or children of its parent.
    void MovePreviousOrNext_(Action action);

    /// Toggles visibility of the RadialMenu for the given Hand. If turning it
    /// on, updates it first from settings.
    void ToggleRadialMenu_(Hand hand);

    /// Convenience to get the current scene.
    SG::Scene & GetScene() const { return *context_->scene_context->scene; }
};

ActionManager::Impl_::Impl_(const ContextPtr &context) : context_(context) {
    ASSERT(context);
    ASSERT(context->scene_context);
    ASSERT(context->tool_context);
    ASSERT(context->command_manager);
    ASSERT(context->name_manager);
    ASSERT(context->panel_manager);
    ASSERT(context->precision_manager);
    ASSERT(context->selection_manager);
    ASSERT(context->settings_manager);
    ASSERT(context->target_manager);
    ASSERT(context->tool_manager);
    ASSERT(context->main_handler);

    const size_t action_count = Util::EnumCount<Action>();
    tooltip_strings_.resize(action_count);
    SetConstantTooltipStrings_();

    // Assume all actions are enabled unless disabled in ProcessUpdate().
    is_action_enabled_.assign(action_count, true);
}

void ActionManager::Impl_::Reset() {
    // Order here matters!
    context_->selection_manager->Reset();
    context_->tool_manager->ResetSession();
    context_->scene_context->root_model->Reset();
    context_->command_manager->ResetCommandList();
    context_->scene_context->tree_panel->Reset();
    context_->name_manager->Reset();
}

void ActionManager::Impl_::UpdateFromSessionState(const SessionState &state) {
    SetToggleState_(Action::kTogglePointTarget, state.IsPointTargetVisible());
    SetToggleState_(Action::kToggleEdgeTarget,  state.IsEdgeTargetVisible());
    SetToggleState_(Action::kToggleShowEdges,   state.AreEdgesShown());
    SetToggleState_(Action::kToggleBuildVolume, state.IsBuildVolumeVisible());
    SetToggleState_(Action::kToggleAxisAligned, state.IsAxisAligned());
}

void ActionManager::Impl_::ProcessUpdate() {
    UpdateEnabledFlags_();
}

std::string ActionManager::Impl_::GetHelpTooltip(Action action) {
    // Always use the constant version of the tooltip for help.
    return tooltip_strings_[Util::EnumInt(action)];
}

std::string ActionManager::Impl_::GetRegularTooltip(Action action) {
    // If there is a context-sensitive version of the tooltip, use it.
    // Otherwise, use the constant one.
    std::string str = GetUpdatedTooltip_(action);
    if (str.empty())
        str = tooltip_strings_[Util::EnumInt(action)];

    // Only Action::kNone is allowed to not have a tooltip.
    ASSERTM(! str.empty() || action == Action::kNone,
            "No tooltip for Action " + Util::EnumName(action));
    return str;
}

bool ActionManager::Impl_::CanApplyAction(Action action) const {
    return is_action_enabled_[Util::EnumInt(action)];
}

void ActionManager::Impl_::ApplyAction(Action action) {
    ASSERT(CanApplyAction(action));

    // Handle toggles specially.
    if (IsToggleAction(action)) {
        SetToggleState_(action, ! GetToggleState_(action));
        return;
    }

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

      case Action::kOpenSessionPanel:
        context_->panel_manager->OpenPanel("SessionPanel");
        break;
      case Action::kOpenSettingsPanel:
        context_->panel_manager->OpenPanel("SettingsPanel");
        break;
      case Action::kOpenInfoPanel:
        OpenInfoPanel_();
        break;
      case Action::kOpenHelpPanel:
        context_->panel_manager->OpenPanel("HelpPanel");
        break;

      case Action::kCreateBox:
        CreatePrimitiveModel_(PrimitiveType::kBox);
        break;
      case Action::kCreateCylinder:
        CreatePrimitiveModel_(PrimitiveType::kCylinder);
        break;
      case Action::kCreateImportedModel:
        CreateImportedModel_();
        break;
      // case Action::kCreateRevSurf: XXXX
      case Action::kCreateSphere:
        CreatePrimitiveModel_(PrimitiveType::kSphere);
        break;
      case Action::kCreateText:
        CreateTextModel_();
        break;
      case Action::kCreateTorus:
        CreatePrimitiveModel_(PrimitiveType::kTorus);
        break;

      case Action::kConvertBevel:
        ConvertModels_(CreateCommand_<ConvertBevelCommand>());
        break;
      // case Action::kConvertClip:
      // case Action::kConvertMirror:

      case Action::kCombineCSGDifference:
        CreateCSGModel_(CSGOperation::kDifference);
        break;
      case Action::kCombineCSGIntersection:
        CreateCSGModel_(CSGOperation::kIntersection);
        break;
      case Action::kCombineCSGUnion:
        CreateCSGModel_(CSGOperation::kUnion);
        break;
      case Action::kCombineHull:
        CreateHullModel_();
        break;

      case Action::kColorTool:
      case Action::kComplexityTool:
      case Action::kNameTool:
      case Action::kRotationTool:
      case Action::kScaleTool:
      case Action::kTranslationTool:
        context_->tool_manager->UseGeneralTool(Util::EnumToWord(action),
                                               GetSelection());
        break;

      case Action::kSwitchToPreviousTool:
        context_->tool_manager->UsePreviousGeneralTool(GetSelection());
        break;
      case Action::kSwitchToNextTool:
        context_->tool_manager->UseNextGeneralTool(GetSelection());
        break;

      case Action::kDecreaseComplexity:
        ChangeComplexity_(-.05f);
        break;
      case Action::kIncreaseComplexity:
        ChangeComplexity_(.05f);
        break;

      case Action::kDecreasePrecision:
        context_->precision_manager->Decrease();
        break;
      case Action::kIncreasePrecision:
        context_->precision_manager->Increase();
        break;

      case Action::kMoveToOrigin:
        MoveSelectionToOrigin_();
        break;

      case Action::kSelectAll:
        context_->selection_manager->SelectAll();
        break;
      case Action::kSelectNone:
        context_->selection_manager->DeselectAll();
        break;

      case Action::kSelectParent:
        context_->selection_manager->SelectInDirection(
            SelectionManager::Direction::kParent);
        break;
      case Action::kSelectFirstChild:
        context_->selection_manager->SelectInDirection(
            SelectionManager::Direction::kFirstChild);
        break;
      case Action::kSelectPreviousSibling:
        context_->selection_manager->SelectInDirection(
            SelectionManager::Direction::kPreviousSibling);
        break;
      case Action::kSelectNextSibling:
        context_->selection_manager->SelectInDirection(
            SelectionManager::Direction::kNextSibling);
        break;

      // case Action::kDelete:
      // case Action::kCut:
      // case Action::kCopy:
      // case Action::kPaste:
      // case Action::kPasteInto:

      // case Action::kLinearLayout:
      // case Action::kRadialLayout:

      case Action::kMovePrevious:
        MovePreviousOrNext_(action);
        break;
      case Action::kMoveNext:
        MovePreviousOrNext_(action);
        break;

      case Action::kHideSelected: {
          const std::vector<ModelPtr> models = GetSelection().GetModels();
          context_->selection_manager->DeselectAll();
          for (const auto &model: models)
              context_->scene_context->root_model->HideModel(model);
          break;
      }
      case Action::kShowAll:
        context_->scene_context->root_model->ShowAllModels();
        break;

#if defined DEBUG
      case Action::kReloadScene:
        ASSERT(reload_func_);
        reload_func_();
        break;
#endif

      default:
        // XXXX Do something for real.
        std::cerr << "XXXX Unimplemented action "
                  << Util::EnumName(action) << "\n";
    }
}

bool ActionManager::Impl_::GetToggleState_(Action action) const {
    const auto &tm = *context_->tool_manager;
    const auto &ss = *context_->command_manager->GetSessionState();

    switch (action) {
      // Tools:
      case Action::kColorTool:
      case Action::kComplexityTool:
      case Action::kNameTool:
      case Action::kRotationTool:
      case Action::kScaleTool:
      case Action::kTranslationTool:
        return ! tm.IsUsingSpecializedTool() &&
            tm.GetCurrentTool()->GetTypeName() == Util::EnumToWord(action);
      case Action::kToggleSpecializedTool:
        return tm.IsUsingSpecializedTool();

      // Other toggles:
      case Action::kTogglePointTarget:
        return ss.IsPointTargetVisible();
      case Action::kToggleEdgeTarget:
        return ss.IsEdgeTargetVisible();
      case Action::kToggleAxisAligned:
        return ss.IsAxisAligned();
      case Action::kToggleInspector:
        return context_->scene_context->inspector->IsEnabled();
      case Action::kToggleBuildVolume:
        return ss.IsBuildVolumeVisible();
      case Action::kToggleShowEdges:
        return ss.AreEdgesShown();
      case Action::kToggleLeftRadialMenu:
        return context_->scene_context->left_radial_menu->IsEnabled();
      case Action::kToggleRightRadialMenu:
        return context_->scene_context->right_radial_menu->IsEnabled();

      default:
        // Anything else is not a toggle.
        ASSERTM(false, Util::EnumName(action) + " is not a toggle");
        return false;
    }
}

void ActionManager::Impl_::SetToggleState_(Action action, bool state) {
    const auto &ss = context_->command_manager->GetSessionState();

    switch (action) {
      case Action::kToggleSpecializedTool:
        context_->tool_manager->ToggleSpecializedTool(GetSelection());
        ASSERT(context_->tool_manager->IsUsingSpecializedTool() == state);
        break;

      case Action::kTogglePointTarget:
        context_->target_manager->SetPointTargetVisible(state);
        ss->SetPointTargetVisible(state);
        break;
      case Action::kToggleEdgeTarget:
        context_->target_manager->SetEdgeTargetVisible(state);
        ss->SetEdgeTargetVisible(state);
        break;

      case Action::kToggleAxisAligned:
        ss->SetAxisAligned(state);
        break;

      case Action::kToggleInspector:
        if (context_->scene_context->inspector->IsEnabled())
            context_->scene_context->inspector->Deactivate();
        else
            context_->scene_context->inspector->Activate(
                GetSelection().GetPrimary().GetModel());
        break;

      case Action::kToggleBuildVolume: {
          const CoordConv cc(context_->scene_context->path_to_stage);
          const Vector3f &size =
              context_->settings_manager->GetSettings().GetBuildVolumeSize();
          const auto &bv = context_->scene_context->build_volume;
          context_->scene_context->root_model->ActivateBuildVolume(
              state, size, cc.GetRootToObjectMatrix());
          if (state) {
              bv->SetScale(size);
              bv->SetTranslation(Vector3f(0, .5f * size[1], 0));
          }
          bv->SetEnabled(state);
          ss->SetBuildVolumeVisible(state);
          break;
      }

      case Action::kToggleShowEdges:
          context_->scene_context->root_model->ShowEdges(state);
          context_->scene_context->inspector->ShowEdges(state);
          ss->SetEdgesShown(state);
          break;

      case Action::kToggleLeftRadialMenu: {
          auto &menu = context_->scene_context->left_radial_menu;
          if (state) {
              const auto &settings = context_->settings_manager->GetSettings();
              menu->UpdateFromInfo(settings.GetLeftRadialMenuInfo());
          }
          menu->SetEnabled(state);
        break;
      }
      case Action::kToggleRightRadialMenu: {
          auto &menu = context_->scene_context->left_radial_menu;
          if (state) {
              const auto &settings = context_->settings_manager->GetSettings();
              menu->UpdateFromInfo(settings.GetLeftRadialMenuInfo());
          }
          menu->SetEnabled(state);
        break;
      }

      default:
        // Anything else is not a toggle.
        ASSERTM(false, Util::EnumName(action) + " is not a toggle");
    }
}

void ActionManager::Impl_::SetConstantTooltipStrings_() {
    auto set_tt = [&](Action action, const std::string &str){
        tooltip_strings_[Util::EnumInt(action)] = str;
    };

    set_tt(Action::kQuit, "Exit the application");
    set_tt(Action::kUndo, "Undo the last command");
    set_tt(Action::kRedo, "Redo the last undone command");

    set_tt(Action::kOpenSessionPanel,
           "Open the panel to save or open session files");
    set_tt(Action::kOpenSettingsPanel, "Edit application settings");
    set_tt(Action::kOpenInfoPanel,
           "Open the panel to show information about selected models");
    set_tt(Action::kOpenHelpPanel, "Open the panel to access help");

    set_tt(Action::kCreateBox,           "Create a primitive Box model");
    set_tt(Action::kCreateCylinder,      "Create a primitive Cylinder model");
    set_tt(Action::kCreateImportedModel, "Import a model from a file");
    set_tt(Action::kCreateRevSurf,
           "Create a model that is a surface of revolution");
    set_tt(Action::kCreateSphere,        "Create a primitive Sphere model");
    set_tt(Action::kCreateText,          "Create a 3D Text model");
    set_tt(Action::kCreateTorus,         "Create a primitive Torus model");

    set_tt(Action::kConvertBevel, "Convert selected models to beveled models");
    set_tt(Action::kConvertClip,  "Convert selected models to clipped models");
    set_tt(Action::kConvertMirror,
           "Convert selected models to mirrored models");

    set_tt(Action::kCombineCSGDifference,
           "Create a CSG Difference from selected objects");
    set_tt(Action::kCombineCSGIntersection,
           "Create a CSG Intersection from selected objects");
    set_tt(Action::kCombineCSGUnion,
           "Create a CSG Union from selected objects");
    set_tt(Action::kCombineHull,
           "Create a model that is the convex hull of selected models");

    set_tt(Action::kColorTool,
           "Edit the color of the selected models");
    set_tt(Action::kComplexityTool,
           "Edit the complexity of the selected models");
    set_tt(Action::kNameTool,
           "Edit the name of the selected model");
    set_tt(Action::kRotationTool,
           "Rotate the selected models (Alt for in-place)");
    set_tt(Action::kScaleTool,
           "Change the size of the selected models (Alt for symmetric)");
    set_tt(Action::kTranslationTool,
           "Change the position of the selected models");

    set_tt(Action::kSwitchToPreviousTool,
           "Switch to the previous general tool");
    set_tt(Action::kSwitchToNextTool,
           "Switch to the next general tool");

    set_tt(Action::kToggleSpecializedTool,
           "TOGGLE: Switch between the current general tool and the"
           " specialized tool for the selected models");

    set_tt(Action::kDecreaseComplexity,
           "Decrease the complexity of the selected models by .05");
    set_tt(Action::kIncreaseComplexity,
           "Increase the complexity of the selected models by .05");

    set_tt(Action::kDecreasePrecision, "Decrease the current precision");
    set_tt(Action::kIncreasePrecision, "Increase the current precision");

    set_tt(Action::kMoveToOrigin, "Move the primary selection to the origin");

    set_tt(Action::kSelectAll,    "Select all top-level models");
    set_tt(Action::kSelectNone,   "Deselect all selected models");
    set_tt(Action::kSelectParent, "Select the parent of the primary selection");
    set_tt(Action::kSelectFirstChild,
           "Select the first child of the primary selection");
    set_tt(Action::kSelectPreviousSibling,
           "Select the previous sibling of the primary selection");
    set_tt(Action::kSelectNextSibling,
           "Select the next sibling of the primary selection");

    set_tt(Action::kDelete, "Delete all selected models");
    set_tt(Action::kCut,    "Cut all selected models to the clipboard");
    set_tt(Action::kCopy,   "Copy all selected models to the clipboard");
    set_tt(Action::kPaste,  "Paste all models from the clipboard");
    set_tt(Action::kPasteInto,
           "Paste all models from the clipboard as children of"
           " the selected model");

    set_tt(Action::kTogglePointTarget,
           "TOGGLE: Activate or deactivate the point target");
    set_tt(Action::kToggleEdgeTarget,
           "TOGGLE: Activate or deactivate the edge target");

    set_tt(Action::kLinearLayout,
           "Lay out the centers of the selected models along a\n"
           "line using the edge target");
    set_tt(Action::kRadialLayout,
           "Lay out selected models along a circular arc");

    set_tt(Action::kToggleAxisAligned,
           "TOGGLE: Transform models in local or global coordinates");

    set_tt(Action::kMovePrevious, "Move the selected model up in the order");
    set_tt(Action::kMoveNext,     "Move the selected model down in the order");

    set_tt(Action::kToggleInspector,
           "TOGGLE: Open or close the Inspector for the"
           " current primary selection");
    set_tt(Action::kToggleBuildVolume,
           "TOGGLE: Show or hide the translucent build volume");
    set_tt(Action::kToggleShowEdges,
           "TOGGLE: Show or hide edges on all models");

    set_tt(Action::kHideSelected, "Hide selected top-level models");
    set_tt(Action::kShowAll,      "Show all hidden top-level models");

    set_tt(Action::kToggleLeftRadialMenu,
           "TOGGLE: Show or hide the left radial menu");
    set_tt(Action::kToggleRightRadialMenu,
           "TOGGLE: Show or hide the right radial menu");
}

std::string ActionManager::Impl_::GetUpdatedTooltip_(Action action) {
    // Helper string when needed.
    std::string s;

    // For switching between "Hide" and "Show".
    auto hide_show = [](bool visible){ return visible ? "Hide" : "Show"; };

    switch (action) {
      case Action::kUndo: {
        auto &cl = *context_->command_manager->GetCommandList();
        return "Undo the last command:\n<" +
            cl.GetCommandToUndo()->GetDescription() + ">";
      }
      case Action::kRedo: {
        auto &cl = *context_->command_manager->GetCommandList();
        return "Redo the last undone command:\n<" +
            cl.GetCommandToRedo()->GetDescription() + ">";
      }

      case Action::kToggleSpecializedTool:
        if (context_->tool_manager->IsUsingSpecializedTool())
            return "Switch back to the current general tool";
        else
            return "Switch to the specialized " +
                context_->tool_manager->GetSpecializedToolForSelection(
                    GetSelection())->GetTypeName();

      case Action::kTogglePointTarget:
        s = context_->target_manager->IsPointTargetVisible() ?
            "Deactivate" : "Activate";
        return s + " the point target";
      case Action::kToggleEdgeTarget:
        s = context_->target_manager->IsEdgeTargetVisible() ?
            "Deactivate" : "Activate";
        return s + " the edge target";

      case Action::kRadialLayout:
        if (GetSelection().GetCount() == 1U)
            return "Move the bottom center of the selected Model\n"
                "to the point target and orient its +Y axis to\n"
                "the point target direction";
        else
            return "Lay out the bottom centers of the selected models\n"
                "along a circular arc using the point target radial layout";

      case Action::kToggleAxisAligned:
        return context_->tool_context->is_axis_aligned ?
            "Transform models in their local coordinates" :
            "Transform models relative to global coordinate axes";

      case Action::kToggleInspector:
        s = context_->scene_context->inspector->IsEnabled() ? "Close" : "Open";
        return s + " the Inspector for the primary selection";

      case Action::kToggleBuildVolume:
        s = hide_show(context_->scene_context->build_volume->IsEnabled());
        return s + " the build volume";
      case Action::kToggleShowEdges:
        s = hide_show(context_->scene_context->root_model->AreEdgesShown());
        return s + " edges on all models";

      case Action::kToggleLeftRadialMenu:
        s = hide_show(context_->scene_context->left_radial_menu->IsEnabled());
        return s + " the left radial menu";
      case Action::kToggleRightRadialMenu:
        s = hide_show(context_->scene_context->right_radial_menu->IsEnabled());
        return s + " the right radial menu";

      default:
        // Everything else will use the constant version.
        return "";
    }
}

void ActionManager::Impl_::UpdateEnabledFlags_() {
    // Update all enabled flags. They are true by default, so anything that is
    // not set here is assumed to always be enabled.

    auto set_enabled = [&](Action action, bool enabled){
        is_action_enabled_[Util::EnumInt(action)] = enabled;
    };

    // Gather some generally useful information.
    const Selection &sel          = GetSelection();
    const bool       any_selected = sel.HasAny();
    const size_t     sel_count    = sel.GetCount();
    const bool       all_valid    = AreAllMeshesValid_(sel);
    const bool       all_top      = AreAllTopLevel_(sel);
    const auto       root_model   = context_->scene_context->root_model;

    set_enabled(Action::kUndo, context_->command_manager->CanUndo());
    set_enabled(Action::kRedo, context_->command_manager->CanRedo());
    set_enabled(Action::kOpenInfoPanel,
                any_selected ||
                context_->target_manager->IsPointTargetVisible() ||
                context_->target_manager->IsEdgeTargetVisible());

    set_enabled(Action::kConvertBevel,  CanConvert_<BeveledModel>(sel));
#if XXXX
    set_enabled(Action::kConvertClip,   CanConvert_<ClippedModel>(sel));
    set_enabled(Action::kConvertMirror, CanConvert_<MirroredModel>(sel));
#endif

    // CSG requires at least 2 models selected, and all must be valid and at
    // the top level.
    const bool can_do_csg = sel_count >= 2U && all_valid && all_top;
    set_enabled(Action::kCombineCSGDifference,   can_do_csg);
    set_enabled(Action::kCombineCSGIntersection, can_do_csg);
    set_enabled(Action::kCombineCSGUnion,        can_do_csg);

    // Convex hull requires at least 2 models or 1 model that is not already a
    // HullModel.
    set_enabled(Action::kCombineHull,
                all_valid &&
                (sel_count >= 2U ||
                 (sel_count == 1U &&
                  ! Util::IsA<HullModel>(sel.GetPrimary().GetModel()))));

    auto enable_tool = [&](Action action){
        const std::string &name = Util::EnumToWord(action);
        set_enabled(action,
                    context_->tool_manager->CanUseGeneralTool(name, sel));
    };
    enable_tool(Action::kColorTool);
    enable_tool(Action::kComplexityTool);
    enable_tool(Action::kNameTool);
    enable_tool(Action::kRotationTool);
    enable_tool(Action::kScaleTool);
    enable_tool(Action::kTranslationTool);

    const bool can_switch_tools =
        context_->main_handler->IsWaiting() && any_selected &&
        ! context_->tool_manager->IsUsingSpecializedTool();
    set_enabled(Action::kSwitchToPreviousTool, can_switch_tools);
    set_enabled(Action::kSwitchToNextTool,     can_switch_tools);

    set_enabled(Action::kToggleSpecializedTool,
                context_->tool_manager->CanUseSpecializedTool(sel));

    const bool can_set_complexity =
        any_selected && sel.GetPrimary().GetModel()->CanSetComplexity();
    set_enabled(Action::kDecreaseComplexity, can_set_complexity);
    set_enabled(Action::kIncreaseComplexity, can_set_complexity);

    set_enabled(Action::kDecreasePrecision,
                context_->precision_manager->CanDecrease());
    set_enabled(Action::kIncreasePrecision,
                context_->precision_manager->CanIncrease());

    set_enabled(Action::kMoveToOrigin, any_selected);

    set_enabled(Action::kSelectAll, root_model->GetChildModelCount() > 0);
    set_enabled(Action::kSelectNone, any_selected);

    auto enable_direction = [&](Action act, SelectionManager::Direction dir){
        set_enabled(act,
                    context_->selection_manager->CanSelectInDirection(dir));
    };
    enable_direction(Action::kSelectParent,
                     SelectionManager::Direction::kParent);
    enable_direction(Action::kSelectFirstChild,
                     SelectionManager::Direction::kFirstChild);
    enable_direction(Action::kSelectPreviousSibling,
                     SelectionManager::Direction::kPreviousSibling);
    enable_direction(Action::kSelectNextSibling,
                     SelectionManager::Direction::kNextSibling);

    const bool can_delete = CanDeleteModels_(sel);
    set_enabled(Action::kDelete, can_delete);
    set_enabled(Action::kCut,    can_delete);

    set_enabled(Action::kCopy, any_selected);

    set_enabled(Action::kPaste, ! clipboard_manager_.Get().empty());
    set_enabled(Action::kPasteInto,
                ! clipboard_manager_.Get().empty() &&
                sel_count == 1U &&
                Util::IsA<CombinedModel>(sel.GetPrimary().GetModel()));

    set_enabled(Action::kLinearLayout, sel_count > 1U &&
                context_->target_manager->IsEdgeTargetVisible());
    set_enabled(Action::kRadialLayout, any_selected &&
                context_->target_manager->IsPointTargetVisible());

    bool can_move_prev;
    bool can_move_next;
    GetMoveFlags_(sel, can_move_prev, can_move_next);
    set_enabled(Action::kMovePrevious, can_move_prev);
    set_enabled(Action::kMoveNext,     can_move_next);

    set_enabled(Action::kToggleInspector, any_selected);

    set_enabled(Action::kHideSelected, any_selected && all_top);
    set_enabled(Action::kShowAll,      root_model->GetHiddenModelCount() > 0);
}

void ActionManager::Impl_::OpenInfoPanel_() {
    auto init_panel = [&](const PanelPtr &panel){
        InfoPanel::Info info;
        info.selection = GetSelection();
        if (context_->target_manager->IsPointTargetVisible())
            info.point_target = &context_->target_manager->GetPointTarget();
        if (context_->target_manager->IsEdgeTargetVisible())
            info.edge_target  = &context_->target_manager->GetEdgeTarget();

        InfoPanel &info_panel = *Util::CastToDerived<InfoPanel>(panel);
        info_panel.SetInfo(info);
    };

    context_->panel_manager->InitAndOpenPanel("InfoPanel", init_panel);
}

void ActionManager::Impl_::CreateCSGModel_(CSGOperation op) {
    auto ccc = CreateCommand_<CreateCSGModelCommand>();
    ccc->SetOperation(op);
    ccc->SetFromSelection(GetSelection());
    context_->command_manager->AddAndDo(ccc);
    context_->tool_manager->UseSpecializedTool(GetSelection());
}

void ActionManager::Impl_::CreateHullModel_() {
    auto chc = CreateCommand_<CreateHullModelCommand>();
    chc->SetFromSelection(GetSelection());
    context_->command_manager->AddAndDo(chc);
}

void ActionManager::Impl_::CreateImportedModel_() {
    auto cic = CreateCommand_<CreateImportedModelCommand>();
    context_->command_manager->AddAndDo(cic);
    context_->tool_manager->UseSpecializedTool(GetSelection());
}

void ActionManager::Impl_::CreatePrimitiveModel_(PrimitiveType type) {
    auto cpc = CreateCommand_<CreatePrimitiveModelCommand>();
    cpc->SetType(type);
    context_->command_manager->AddAndDo(cpc);
    context_->tool_manager->UseSpecializedTool(GetSelection());
}

void ActionManager::Impl_::CreateTextModel_() {
    auto ctc = CreateCommand_<CreateTextModelCommand>();
    ctc->SetText("A");
    context_->command_manager->AddAndDo(ctc);
    context_->tool_manager->UseSpecializedTool(GetSelection());
}

void ActionManager::Impl_::ConvertModels_(const ConvertCommandPtr &command) {
    command->SetFromSelection(GetSelection());
    context_->command_manager->AddAndDo(command);
    context_->tool_manager->UseSpecializedTool(GetSelection());
}

void ActionManager::Impl_::ChangeComplexity_(float delta) {
    const Selection &sel = GetSelection();
    ASSERT(sel.HasAny());
    const Model &primary = *sel.GetPrimary().GetModel();
    ASSERT(primary.CanSetComplexity());

    const float cur_complexity = primary.GetComplexity();
    const float new_complexity = Clamp(cur_complexity + delta, 0, 1);

    if (new_complexity != cur_complexity) {
        auto ccc = CreateCommand_<ChangeComplexityCommand>();
        ccc->SetFromSelection(sel);
        ccc->SetNewComplexity(new_complexity);
        context_->command_manager->AddAndDo(ccc);
    }
}

void ActionManager::Impl_::MoveSelectionToOrigin_() {
    const Selection &sel = GetSelection();
    ASSERT(sel.HasAny());

    // Get the bottom center of the primary selection in stage coordinates.
    const Model &primary = *sel.GetPrimary().GetModel();
    const Matrix4f owm = CoordConv(sel.GetPrimary()).GetObjectToRootMatrix();
    const Matrix4f wsm = CoordConv(
        context_->scene_context->path_to_stage).GetRootToObjectMatrix();
    const Point3f bottom_center =
        (owm * wsm) * primary.GetBounds().GetFaceCenter(Bounds::Face::kBottom);

    // Get the vector from the bottom center to the origin.
    const Vector3f vec = Point3f::Zero() - bottom_center;

    // Already at the origin? Do nothing.
    if (ion::math::Length(vec) >= .00001f) {
        auto tc = CreateCommand_<TranslateCommand>();
        tc->SetFromSelection(sel);
        tc->SetTranslation(vec);
        context_->command_manager->AddAndDo(tc);
    }
}

void ActionManager::Impl_::MovePreviousOrNext_(Action action) {
    const Selection &sel = GetSelection();
    ASSERT(sel.GetCount() == 1);
    auto coc = CreateCommand_<ChangeOrderCommand>();
    coc->SetFromSelection(sel);
    coc->SetIsPrevious(action == Action::kMovePrevious);
    context_->command_manager->AddAndDo(coc);
}

void ActionManager::Impl_::ToggleRadialMenu_(Hand hand) {
    auto &menu = hand == Hand::kLeft ?
        context_->scene_context->left_radial_menu :
        context_->scene_context->right_radial_menu;
    if (menu->IsEnabled()) {
        menu->SetEnabled(false);
    }
    else {
        const Settings &settings = context_->settings_manager->GetSettings();
        menu->UpdateFromInfo(hand == Hand::kLeft ?
                             settings.GetLeftRadialMenuInfo() :
                             settings.GetRightRadialMenuInfo());
        menu->SetEnabled(true);
    }
}

// ----------------------------------------------------------------------------
// ActionManager functions.
// ----------------------------------------------------------------------------

ActionManager::ActionManager(const ContextPtr &context) :
    impl_(new Impl_(context)) {
}

ActionManager::~ActionManager() {
}

void ActionManager::Reset() {
    impl_->Reset();
}

void ActionManager::UpdateFromSessionState(const SessionState &state) {
    impl_->UpdateFromSessionState(state);
}

void ActionManager::SetReloadFunc(const ReloadFunc &func) {
    impl_->SetReloadFunc(func);
}

void ActionManager::ProcessUpdate() {
    impl_->ProcessUpdate();
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

bool ActionManager::GetToggleState(Action action) const {
    return impl_->GetToggleState(action);
}

bool ActionManager::ShouldQuit() const {
    return impl_->ShouldQuit();
}
