#pragma once

#include <memory>
#include <string>

#include "Enums/Action.h"
#include "Tools/Tool.h"

DECL_SHARED_PTR(ActionManager);
DECL_SHARED_PTR(CommandManager);
DECL_SHARED_PTR(MainHandler);
DECL_SHARED_PTR(NameManager);
DECL_SHARED_PTR(PanelManager);
DECL_SHARED_PTR(PrecisionManager);
DECL_SHARED_PTR(SceneContext);
DECL_SHARED_PTR(SelectionManager);
DECL_SHARED_PTR(SettingsManager);
DECL_SHARED_PTR(TargetManager);
DECL_SHARED_PTR(ToolManager);

/// ActionManager is responsible for applying actions.
///
/// \ingroup Managers
class ActionManager {
  public:
    typedef std::function<void()> ReloadFunc;

    /// The ActionManager::Context stores everything the ActionManager needs to
    /// apply actions.
    struct Context {
        /// The SceneContext storing important scene contents.
        SceneContextPtr     scene_context;

        /// Tool Context.
        Tool::ContextPtr    tool_context;

        // Managers.
        CommandManagerPtr   command_manager;
        NameManagerPtr      name_manager;
        PanelManagerPtr     panel_manager;
        PrecisionManagerPtr precision_manager;
        SelectionManagerPtr selection_manager;
        SettingsManagerPtr  settings_manager;
        TargetManagerPtr    target_manager;
        ToolManagerPtr      tool_manager;

        // Handlers.
        MainHandlerPtr      main_handler;
    };
    typedef std::shared_ptr<Context> ContextPtr;

    /// Initializes the ActionManager with a Context.
    ActionManager(const ContextPtr &context);

    ~ActionManager();

    /// Resets to original conditions.
    void Reset();

    /// Updates from the given SessionState instance.
    void UpdateFromSessionState(const SessionState &state);

    /// Sets a function to call to reload the scene (works only in debug build).
    void SetReloadFunc(const ReloadFunc &func);

    /// Updates state for the current frame. This sets up everything for action
    /// processing enabling.
    void ProcessUpdate();

    /// Returns the tooltip string for an action. If for_help is true, the
    /// string will be a generic description for a help display.
    std::string GetActionTooltip(Action action, bool for_help = false);

    /// Returns true if the given Action can be applied.
    bool CanApplyAction(Action action) const;

    /// Applies the given Action. Asserts if the Action cannot be applied.
    void ApplyAction(Action action);

    /// Returns the current state of the given Action, which must represent a
    /// toggle of some sort.
    bool GetToggleState(Action action) const;

    /// Returns true if the application should exit.
    bool ShouldQuit() const;

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;
};
