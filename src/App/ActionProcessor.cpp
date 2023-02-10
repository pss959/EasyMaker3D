#include "App/ActionProcessor.h"

#include <unordered_set>

#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

#include "Base/HelpMap.h"
#include "Commands/ChangeComplexityCommand.h"
#include "Commands/ChangeOrderCommand.h"
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
#include "Commands/TranslateCommand.h"
#include "Enums/Hand.h"
#include "Enums/PrimitiveType.h"
#include "Handlers/MainHandler.h"
#include "Items/BuildVolume.h"
#include "Items/Inspector.h"
#include "Items/PrecisionControl.h"
#include "Items/RadialMenu.h"
#include "Items/SessionState.h"
#include "Items/Settings.h"
#include "Managers/BoardManager.h"
#include "Managers/ClipboardManager.h"
#include "Managers/CommandManager.h"
#include "Managers/NameManager.h"
#include "Managers/SceneContext.h"
#include "Managers/SelectionManager.h"
#include "Managers/SettingsManager.h"
#include "Managers/TargetManager.h"
#include "Managers/ToolManager.h"
#include "Models/BeveledModel.h"
#include "Models/ClippedModel.h"
#include "Models/CombinedModel.h"
#include "Models/HullModel.h"
#include "Models/MirroredModel.h"
#include "Panels/Board.h"
#include "Panels/InfoPanel.h"
#include "Panels/Panel.h"
#include "Panels/TreePanel.h"
#include "Parser/Registry.h"
#include "Place/EdgeTarget.h"
#include "Place/PrecisionStore.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "Selection/SelPath.h"
#include "Selection/Selection.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/General.h"
#include "Util/KLog.h"

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

}  // anonymous namespace

// ----------------------------------------------------------------------------
// ActionProcessor::Impl_ class.
// ----------------------------------------------------------------------------

class ActionProcessor::Impl_ {
  public:
    Impl_(const ContextPtr &context);

    void Reset();
    void UpdateFromSessionState(const SessionState &state);
    void SetQuitFunc(const QuitFunc &func) { quit_func_ = func; }
    void SetReloadFunc(const ReloadFunc &func) { reload_func_ = func; }
    void ProcessUpdate();
    std::string GetHelpTooltip(Action action);
    std::string GetRegularTooltip(Action action);
    bool CanApplyAction(Action action) const;
    void ApplyAction(Action action);
    bool GetToggleState(Action action) const { return GetToggleState_(action); }

  private:
    ContextPtr            context_;
    QuitFunc              quit_func_;
    ReloadFunc            reload_func_;

    /// Stores a tooltip help strings for each Action.
    HelpMap               help_map_;

    /// Flag for each Action that indicates whether it can be applied.
    std::vector<bool>        is_action_enabled_;

    /// Saves the selection while the Inspector is active.
    Selection                saved_selection_for_inspector_;

    /// Returns the current state for a toggle Action.
    bool GetToggleState_(Action action) const;

    /// Changes the current state for a toggle Action.
    void SetToggleState_(Action action, bool state);

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

    /// Opens the named application Panel.
    void OpenAppPanel_(const std::string &name);

    /// Deletes the current selection.
    void DeleteSelection_();

    /// Copies the current selection to the clipboard.
    void CopySelection_();

    /// Performs a Paste or PasteInto operation.
    void PasteFromClipboard_(bool is_into);

    /// Performs linear layout of selected Models using the EdgeTarget length.
    void DoLinearLayout_();

    /// Performs radial layout of selected Models using the PointTarget.
    void DoRadialLayout_();

    /// \name Model Creation
    /// Each of these adds a Command to create a Model of some type.
    ///@{
    void CreateCSGModel_(CSGOperation op);
    void CreateHullModel_();
    void CreateImportedModel_();
    void CreateRevSurfModel_();
    void CreatePrimitiveModel_(PrimitiveType type);
    void CreateTextModel_();
    ///@}

    /// Adds the given ConvertCommand to convert the current selected Models.
    void ConvertModels_(const ConvertCommandPtr &command);

    /// Modifies the complexity of all selected models by the given amount.
    void ChangeComplexity_(float delta);

    /// Moves the bottom center of the primary Model is at the origin and all
    /// other selected Models by the same amount.
    void MoveSelectionToOrigin_();

    /// Shows or hides the Inspector according to the given flag.
    void ShowInspector_(bool show);

    /// Toggles visibility of the RadialMenu for the given Hand. If turning it
    /// on, updates it first from settings.
    void ToggleRadialMenu_(Hand hand);

    /// Convenience to get the current scene.
    SG::Scene & GetScene() const { return *context_->scene_context->scene; }
};

ActionProcessor::Impl_::Impl_(const ContextPtr &context) : context_(context) {
    ASSERT(context);
    ASSERT(context->scene_context);
    ASSERT(context->board_manager);
    ASSERT(context->clipboard_manager);
    ASSERT(context->command_manager);
    ASSERT(context->name_manager);
    ASSERT(context->precision_store);
    ASSERT(context->selection_manager);
    ASSERT(context->settings_manager);
    ASSERT(context->target_manager);
    ASSERT(context->tool_manager);
    ASSERT(context->main_handler);

    // Assume all actions are enabled unless disabled in ProcessUpdate().
    const size_t action_count = Util::EnumCount<Action>();
    is_action_enabled_.assign(action_count, true);
    is_action_enabled_[Util::EnumInt(Action::kNone)] = false;  // Except this.
}

void ActionProcessor::Impl_::Reset() {
    // Order here matters!
    context_->selection_manager->Reset();
    context_->tool_manager->ResetSession();
    context_->scene_context->root_model->Reset();
    context_->command_manager->ResetCommandList();
    context_->scene_context->tree_panel->Reset();
    context_->name_manager->Reset();
}

void ActionProcessor::Impl_::UpdateFromSessionState(const SessionState &state) {
    SetToggleState_(Action::kTogglePointTarget, state.IsPointTargetVisible());
    SetToggleState_(Action::kToggleEdgeTarget,  state.IsEdgeTargetVisible());
    SetToggleState_(Action::kToggleShowEdges,   state.AreEdgesShown());
    SetToggleState_(Action::kToggleBuildVolume, state.IsBuildVolumeVisible());
    SetToggleState_(Action::kToggleAxisAligned, state.IsAxisAligned());
}

void ActionProcessor::Impl_::ProcessUpdate() {
    UpdateEnabledFlags_();
}

std::string ActionProcessor::Impl_::GetHelpTooltip(Action action) {
    return help_map_.GetHelpString(action);
}

std::string ActionProcessor::Impl_::GetRegularTooltip(Action action) {
    // If there is a context-sensitive version of the tooltip, use it.
    // Otherwise, use the help string.
    std::string str = GetUpdatedTooltip_(action);
    if (str.empty())
        str = GetHelpTooltip(action);

    // Only Action::kNone is allowed to not have a tooltip.
    ASSERTM(! str.empty() || action == Action::kNone,
            "No tooltip for Action " + Util::EnumName(action));
    return str;
}

bool ActionProcessor::Impl_::CanApplyAction(Action action) const {
    return is_action_enabled_[Util::EnumInt(action)];
}

void ActionProcessor::Impl_::ApplyAction(Action action) {
    ASSERTM(CanApplyAction(action), Util::EnumName(action));

    KLOG('j', "Applying action " << Util::EnumName(action));

    // Handle toggles specially.
    if (IsToggleAction(action)) {
        SetToggleState_(action, ! GetToggleState_(action));
        return;
    }

    switch (action) {
      case Action::kUndo:
        context_->command_manager->Undo();
        break;
      case Action::kRedo:
        context_->command_manager->Redo();
        break;
      case Action::kQuit:
        if (quit_func_)
            quit_func_();
        break;

      case Action::kOpenSessionPanel:
        OpenAppPanel_("SessionPanel");
        break;
      case Action::kOpenSettingsPanel:
        OpenAppPanel_("SettingsPanel");
        break;
      case Action::kOpenInfoPanel:
        OpenInfoPanel_();
        break;
      case Action::kOpenHelpPanel:
        OpenAppPanel_("HelpPanel");
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
      case Action::kCreateRevSurf:
        CreateRevSurfModel_();
        break;
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
      case Action::kConvertClip:
        ConvertModels_(CreateCommand_<ConvertClipCommand>());
        break;
      case Action::kConvertMirror:
        ConvertModels_(CreateCommand_<ConvertMirrorCommand>());
        break;

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
      case Action::kIncreasePrecision: {
          auto &pm = *context_->precision_store;
          if (action == Action::kIncreasePrecision)
              pm.Increase();
          else
              pm.Decrease();
          context_->scene_context->precision_control->Update(
              pm.GetLinearPrecision(), pm.GetAngularPrecision());
          break;
      }

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

      case Action::kDelete:
        DeleteSelection_();
        break;
      case Action::kCut:
        CopySelection_();
        DeleteSelection_();
        break;
      case Action::kCopy:
        CopySelection_();
        break;
      case Action::kPaste:
        PasteFromClipboard_(false);
        break;
      case Action::kPasteInto:
        PasteFromClipboard_(true);
        break;

      case Action::kLinearLayout:
        DoLinearLayout_();
        break;
      case Action::kRadialLayout:
        DoRadialLayout_();
        break;

      case Action::kMovePrevious:
        context_->scene_context->tree_panel->MoveUpOrDown(true);
        break;
      case Action::kMoveNext:
        context_->scene_context->tree_panel->MoveUpOrDown(false);
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

#if ! RELEASE_BUILD
      case Action::kReloadScene:
        ASSERT(reload_func_);
        reload_func_();
        break;
#endif

      default:
        ASSERTM(false, "Unimplemented action " + Util::EnumName(action));
    }
}

bool ActionProcessor::Impl_::GetToggleState_(Action action) const {
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

void ActionProcessor::Impl_::SetToggleState_(Action action, bool state) {
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
        // Reselect so that attached tools use the correct alignment.
        context_->selection_manager->ReselectAll();
        break;

      case Action::kToggleInspector:
        ShowInspector_(! context_->scene_context->inspector->IsEnabled());
        break;

      case Action::kToggleBuildVolume:
        context_->scene_context->build_volume->Activate(state);
        ss->SetBuildVolumeVisible(state);
        break;

      case Action::kToggleShowEdges:
          context_->scene_context->root_model->ShowEdges(state);
          context_->scene_context->inspector->ShowEdges(state);
          ss->SetEdgesShown(state);
          break;

      case Action::kToggleLeftRadialMenu: {
          auto &menu = context_->scene_context->left_radial_menu;
          if (state) {
              const auto &settings = context_->settings_manager->GetSettings();
              menu->UpdateFromInfo(settings.GetRadialMenuInfo(Hand::kLeft));
          }
          menu->SetEnabled(state);
        break;
      }
      case Action::kToggleRightRadialMenu: {
          auto &menu = context_->scene_context->right_radial_menu;
          if (state) {
              const auto &settings = context_->settings_manager->GetSettings();
              menu->UpdateFromInfo(settings.GetRadialMenuInfo(Hand::kRight));
          }
          menu->SetEnabled(state);
        break;
      }

      default:
        // Anything else is not a toggle.
        ASSERTM(false, Util::EnumName(action) + " is not a toggle");
    }
}

std::string ActionProcessor::Impl_::GetUpdatedTooltip_(Action action) {
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
        return context_->command_manager->GetSessionState()->IsAxisAligned() ?
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
        // Everything else will use the help string.
        return "";
    }
}

void ActionProcessor::Impl_::UpdateEnabledFlags_() {
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
    const auto       &sc          = *context_->scene_context;
    const auto       root_model   = sc.root_model;

    set_enabled(Action::kUndo, context_->command_manager->CanUndo());
    set_enabled(Action::kRedo, context_->command_manager->CanRedo());

    // Panels cannot be opened if the AppBoard is already in use.
    const bool can_open_app_panel = ! sc.app_board->IsShown();
    set_enabled(Action::kOpenSessionPanel,  can_open_app_panel);
    set_enabled(Action::kOpenSettingsPanel, can_open_app_panel);
    set_enabled(Action::kOpenInfoPanel,
                can_open_app_panel &&
                (any_selected ||
                 context_->target_manager->IsPointTargetVisible() ||
                 context_->target_manager->IsEdgeTargetVisible()));
    set_enabled(Action::kOpenHelpPanel,     can_open_app_panel);

    set_enabled(Action::kConvertBevel,  CanConvert_<BeveledModel>(sel));
    set_enabled(Action::kConvertClip,   CanConvert_<ClippedModel>(sel));
    set_enabled(Action::kConvertMirror, CanConvert_<MirroredModel>(sel));

    // CSG requires at least 2 models selected, and all must be valid and at
    // the top level.
    const bool can_do_csg = sel_count >= 2U && all_valid && all_top;
    set_enabled(Action::kCombineCSGDifference,   can_do_csg);
    set_enabled(Action::kCombineCSGIntersection, can_do_csg);
    set_enabled(Action::kCombineCSGUnion,        can_do_csg);

    // Convex hull requires at least 2 models or 1 model that is not already a
    // HullModel. Note that invalid models can be handled properly.
    set_enabled(Action::kCombineHull,
                sel_count >= 2U ||
                (sel_count == 1U &&
                 ! Util::IsA<HullModel>(sel.GetPrimary().GetModel())));

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
                context_->precision_store->CanDecrease());
    set_enabled(Action::kIncreasePrecision,
                context_->precision_store->CanIncrease());

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

    set_enabled(Action::kPaste, ! context_->clipboard_manager->Get().empty());
    set_enabled(Action::kPasteInto,
                ! context_->clipboard_manager->Get().empty() &&
                sel_count == 1U &&
                Util::IsA<CombinedModel>(sel.GetPrimary().GetModel()));

    set_enabled(Action::kLinearLayout, sel_count > 1U &&
                context_->target_manager->IsEdgeTargetVisible());
    set_enabled(Action::kRadialLayout, any_selected &&
                context_->target_manager->IsPointTargetVisible());

    set_enabled(Action::kMovePrevious, sc.tree_panel->CanMoveUpOrDown(true));
    set_enabled(Action::kMoveNext,     sc.tree_panel->CanMoveUpOrDown(false));

    set_enabled(Action::kToggleInspector, any_selected);

    set_enabled(Action::kHideSelected, any_selected && all_top);
    set_enabled(Action::kShowAll,      root_model->GetHiddenModelCount() > 0);

    const bool menus_enabled =
        context_->settings_manager->GetSettings().GetRadialMenusMode() !=
        RadialMenusMode::kDisabled;
    set_enabled(Action::kToggleLeftRadialMenu,  menus_enabled);
    set_enabled(Action::kToggleRightRadialMenu, menus_enabled);
}

void ActionProcessor::Impl_::OpenInfoPanel_() {
    InfoPanel::Info info;
    info.selection = GetSelection();
    if (context_->target_manager->IsPointTargetVisible())
        info.point_target = &context_->target_manager->GetPointTarget();
    if (context_->target_manager->IsEdgeTargetVisible())
        info.edge_target  = &context_->target_manager->GetEdgeTarget();

    auto ip = context_->board_manager->GetTypedPanel<InfoPanel>("InfoPanel");
    ip->SetInfo(info);

    OpenAppPanel_("InfoPanel");
}

void ActionProcessor::Impl_::OpenAppPanel_(const std::string &name) {
    auto  panel = context_->board_manager->GetPanel(name);
    auto &board = context_->scene_context->app_board;
    board->SetPanel(panel);
    context_->board_manager->ShowBoard(board, true);
}

void ActionProcessor::Impl_::DeleteSelection_() {
    auto dc = CreateCommand_<DeleteCommand>();
    dc->SetFromSelection(GetSelection());
    context_->command_manager->AddAndDo(dc);
}

void ActionProcessor::Impl_::CopySelection_() {
    auto cc = CreateCommand_<CopyCommand>();
    cc->SetFromSelection(GetSelection());
    context_->command_manager->AddAndDo(cc);
}

void ActionProcessor::Impl_::PasteFromClipboard_(bool is_into) {
    auto pc = CreateCommand_<PasteCommand>();
    if (is_into) {
        // The target of the paste-into is the current selection, which must be
        // a ParentModel.
        const Selection &sel = GetSelection();
        ASSERT(sel.HasAny());
        const auto &model = sel.GetPrimary().GetModel();
        ASSERT(Util::IsA<ParentModel>(model));
        pc->SetParentName(model->GetName());
    }
    context_->command_manager->AddAndDo(pc);
}

void ActionProcessor::Impl_::DoLinearLayout_() {
    const Selection &sel = GetSelection();
    ASSERT(sel.GetCount() > 1U);

    // Create and execute a command to lay out the Models.
    auto llc = CreateCommand_<LinearLayoutCommand>();
    llc->SetFromSelection(sel);
    llc->SetFromTarget(context_->target_manager->GetEdgeTarget());

    context_->command_manager->AddAndDo(llc);
}

void ActionProcessor::Impl_::DoRadialLayout_() {
    const Selection &sel = GetSelection();
    ASSERT(sel.HasAny());

    // Create and execute a command to lay out the Models.
    auto rlc = CreateCommand_<RadialLayoutCommand>();
    rlc->SetFromSelection(sel);
    rlc->SetFromTarget(context_->target_manager->GetPointTarget());

    context_->command_manager->AddAndDo(rlc);
}

void ActionProcessor::Impl_::CreateCSGModel_(CSGOperation op) {
    auto ccc = CreateCommand_<CreateCSGModelCommand>();
    ccc->SetOperation(op);
    ccc->SetFromSelection(GetSelection());
    context_->command_manager->AddAndDo(ccc);
    context_->tool_manager->UseSpecializedTool(GetSelection());
}

void ActionProcessor::Impl_::CreateHullModel_() {
    auto chc = CreateCommand_<CreateHullModelCommand>();
    chc->SetFromSelection(GetSelection());
    context_->command_manager->AddAndDo(chc);
}

void ActionProcessor::Impl_::CreateImportedModel_() {
    auto cic = CreateCommand_<CreateImportedModelCommand>();
    context_->command_manager->AddAndDo(cic);
    context_->tool_manager->UseSpecializedTool(GetSelection());
}

void ActionProcessor::Impl_::CreateRevSurfModel_() {
    auto crc = CreateCommand_<CreateRevSurfModelCommand>();
    context_->command_manager->AddAndDo(crc);
    context_->tool_manager->UseSpecializedTool(GetSelection());
}

void ActionProcessor::Impl_::CreatePrimitiveModel_(PrimitiveType type) {
    auto cpc = CreateCommand_<CreatePrimitiveModelCommand>();
    cpc->SetType(type);
    context_->command_manager->AddAndDo(cpc);
    context_->tool_manager->UseSpecializedTool(GetSelection());
}

void ActionProcessor::Impl_::CreateTextModel_() {
    auto ctc = CreateCommand_<CreateTextModelCommand>();
    ctc->SetText("A");
    context_->command_manager->AddAndDo(ctc);
    context_->tool_manager->UseSpecializedTool(GetSelection());
}

void ActionProcessor::Impl_::ConvertModels_(const ConvertCommandPtr &command) {
    command->SetFromSelection(GetSelection());
    context_->command_manager->AddAndDo(command);
    context_->tool_manager->UseSpecializedTool(GetSelection());
}

void ActionProcessor::Impl_::ChangeComplexity_(float delta) {
    const Selection &sel = GetSelection();
    ASSERT(sel.HasAny());
    const Model &primary = *sel.GetPrimary().GetModel();
    ASSERT(primary.CanSetComplexity());

    const float cur_complexity = primary.GetComplexity();
    const float new_complexity = Clamp(cur_complexity + delta, 0.f, 1.f);

    if (new_complexity != cur_complexity) {
        auto ccc = CreateCommand_<ChangeComplexityCommand>();
        ccc->SetFromSelection(sel);
        ccc->SetNewComplexity(new_complexity);
        context_->command_manager->AddAndDo(ccc);
    }
}

void ActionProcessor::Impl_::MoveSelectionToOrigin_() {
    const Selection &sel = GetSelection();
    ASSERT(sel.HasAny());
    const Model &primary = *sel.GetPrimary().GetModel();

    // Compute the bounds in stage coordinates.
    const Matrix4f osm =
        SG::CoordConv(sel.GetPrimary()).GetObjectToRootMatrix();
    const Bounds stage_bounds = TransformBounds(primary.GetBounds(), osm);

    // Compute the translation to put the bounds center at 0 in X and Z and to
    // put the lowest point on the bounds on the stage.
    const Point3f stage_center = stage_bounds.GetCenter();
    const Vector3f trans(-stage_center[0],
                         -stage_bounds.GetMinPoint()[1],
                         -stage_center[2]);

    // Already at the origin? Do nothing.
    if (ion::math::Length(trans) >= .00001f) {
        auto tc = CreateCommand_<TranslateCommand>();
        tc->SetFromSelection(sel);
        tc->SetTranslation(trans);
        context_->command_manager->AddAndDo(tc);
    }
}

void ActionProcessor::Impl_::ShowInspector_(bool show) {
    auto &inspector = *context_->scene_context->inspector;
    if (show) {
        // Get the controller, if any, that caused the inspector to be shown.
        const auto dev = context_->main_handler->GetLastActiveDevice();
        ControllerPtr controller;
        if (dev == Event::Device::kLeftController)
            controller = context_->scene_context->left_controller;
        else if (dev == Event::Device::kRightController)
            controller = context_->scene_context->right_controller;

        // Deselect everything after saving the selection.
        saved_selection_for_inspector_ = GetSelection();
        context_->selection_manager->DeselectAll();

        // Make sure the selected Model is visible. (This is needed in case it
        // is a child Model.)
        const auto &primary =
            saved_selection_for_inspector_.GetPrimary().GetModel();
        primary->SetEnabled(true);

        inspector.Activate(primary, controller);
        inspector.SetDeactivationFunc([&](){ ShowInspector_(false); });
    }
    else {
        // This may be called by the Inspector, so see if it is already
        // deactivated.
        if (inspector.IsEnabled())
            inspector.Deactivate();
        context_->selection_manager->ChangeSelection(
            saved_selection_for_inspector_);
        inspector.SetDeactivationFunc(nullptr);
    }
}

void ActionProcessor::Impl_::ToggleRadialMenu_(Hand hand) {
    auto &menu = hand == Hand::kLeft ?
        context_->scene_context->left_radial_menu :
        context_->scene_context->right_radial_menu;
    if (menu->IsEnabled()) {
        menu->SetEnabled(false);
    }
    else {
        const Settings &settings = context_->settings_manager->GetSettings();
        menu->UpdateFromInfo(settings.GetRadialMenuInfo(hand));
        menu->SetEnabled(true);
    }
}

// ----------------------------------------------------------------------------
// ActionProcessor functions.
// ----------------------------------------------------------------------------

ActionProcessor::ActionProcessor(const ContextPtr &context) :
    impl_(new Impl_(context)) {
}

ActionProcessor::~ActionProcessor() {
}

void ActionProcessor::Reset() {
    impl_->Reset();
}

void ActionProcessor::UpdateFromSessionState(const SessionState &state) {
    impl_->UpdateFromSessionState(state);
}

void ActionProcessor::SetQuitFunc(const QuitFunc &func) {
    impl_->SetQuitFunc(func);
}

void ActionProcessor::SetReloadFunc(const ReloadFunc &func) {
    impl_->SetReloadFunc(func);
}

void ActionProcessor::ProcessUpdate() {
    impl_->ProcessUpdate();
}

std::string ActionProcessor::GetActionTooltip(Action action, bool for_help) {
    return for_help ? impl_->GetHelpTooltip(action) :
        impl_->GetRegularTooltip(action);
}

bool ActionProcessor::CanApplyAction(Action action) const {
    return impl_->CanApplyAction(action);
}

void ActionProcessor::ApplyAction(Action action) {
    impl_->ApplyAction(action);
}

bool ActionProcessor::GetToggleState(Action action) const {
    return impl_->GetToggleState(action);
}
