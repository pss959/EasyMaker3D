#pragma once

#include <memory>

#include "Enums/Action.h"
#include "Handlers/MainHandler.h"
#include "Managers/CommandManager.h"
#include "Managers/SelectionManager.h"
// #include "Managers/TargetManager.h"
#include "Managers/ToolManager.h"
#include "SG/Typedefs.h"

/// ActionManager is responsible for applying actions.
///
/// \ingroup Managers
class ActionManager {
  public:
    /// The ActionManager::Context stores everything the ActionManager needs to
    /// apply actions.
    struct Context {
        /// The current scene.
        SG::ScenePtr        scene;

        // Managers.
        CommandManagerPtr   command_manager;
        SelectionManagerPtr selection_manager;
        // TargetManagerPtr    target_manager;
        ToolManagerPtr      tool_manager;

        // Handlers.
        MainHandlerPtr      main_handler;
    };

    /// Initializes the ActionManager with a Context.
    ActionManager(const Context &context);

    ~ActionManager();

    /// Returns true if the given Action can be applied.
    bool CanApplyAction(Action action) const;

    /// Applies the given Action. Asserts if the Action cannot be applied.
    void ApplyAction(Action action);

    /// Returns true if the application should exit.
    bool ShouldQuit() const;

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;
};

typedef std::shared_ptr<ActionManager> ActionManagerPtr;
