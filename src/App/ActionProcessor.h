#pragma once

#include <functional>

#include "Agents/ActionAgent.h"
#include "Enums/Action.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(ActionProcessor);
DECL_SHARED_PTR(BoardManager);
DECL_SHARED_PTR(ClipboardManager);
DECL_SHARED_PTR(CommandManager);
DECL_SHARED_PTR(MainHandler);
DECL_SHARED_PTR(NameManager);
DECL_SHARED_PTR(PrecisionStore);
DECL_SHARED_PTR(SceneContext);
DECL_SHARED_PTR(SelectionManager);
DECL_SHARED_PTR(SessionState);
DECL_SHARED_PTR(SettingsManager);
DECL_SHARED_PTR(TargetManager);
DECL_SHARED_PTR(ToolBox);

/// ActionProcessor is responsible for applying actions.
///
/// \ingroup App
class ActionProcessor : public ActionAgent {
  public:
    using QuitFunc   = std::function<void()>;
    using ReloadFunc = std::function<void()>;

    /// The ActionProcessor::Context stores everything the ActionProcessor
    /// needs to apply actions.
    struct Context {
        /// The SceneContext storing important scene contents.
        SceneContextPtr     scene_context;

        // Managers.
        BoardManagerPtr     board_manager;
        ClipboardManagerPtr clipboard_manager;
        CommandManagerPtr   command_manager;
        NameManagerPtr      name_manager;
        PrecisionStorePtr   precision_store;
        SelectionManagerPtr selection_manager;
        SettingsManagerPtr  settings_manager;
        TargetManagerPtr    target_manager;
        ToolBoxPtr          tool_box;

        // Handlers.
        MainHandlerPtr      main_handler;
    };
    DECL_SHARED_PTR(Context);

    /// Initializes the ActionProcessor with a Context and a function to call
    /// when the user tries to quit.
    ActionProcessor(const ContextPtr &context);

    ~ActionProcessor();

    /// Resets to original conditions.
    virtual void Reset() override;

    /// Updates from the given SessionState instance.
    virtual void UpdateFromSessionState(const SessionState &state) override;

    /// Sets a function to call when the user tries to quit.
    void SetQuitFunc(const QuitFunc &func);

    /// Sets a function to call to reload the scene (works only in debug build).
    void SetReloadFunc(const ReloadFunc &func);

    /// Updates state for the current frame. This sets up everything for action
    /// processing enabling.
    void ProcessUpdate();

    /// Returns the tooltip string for an action. If for_help is true, the
    /// string will be a generic description for a help display.
    Str GetActionTooltip(Action action, bool for_help = false);

    /// Returns true if the given Action can be applied.
    virtual bool CanApplyAction(Action action) const override;

    /// Applies the given Action. Asserts if the Action cannot be applied.
    virtual void ApplyAction(Action action) override;

    /// Returns the current state of the given Action, which must represent a
    /// toggle of some sort.
    bool GetToggleState(Action action) const;

    /// Shows or hides the given Model, updating the SessionState as well. If
    /// \p model is null, this changes all top-level Models.
    virtual void SetModelVisibility(const ModelPtr &model,
                                    bool is_visible) override;

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;
};
