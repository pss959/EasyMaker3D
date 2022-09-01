#pragma once

#include "Base/Memory.h"

DECL_SHARED_PTR(CommandManager);
DECL_SHARED_PTR(SceneContext);
DECL_SHARED_PTR(SelectionManager);
DECL_SHARED_PTR(SessionManager);

/// This struct defines a context that can be provided to unit tests. More can
/// be added to it as necessary. The Application class sets up an instance on
/// demand.
struct TestContext {
    // Managers
    CommandManagerPtr   command_manager;
    SelectionManagerPtr selection_manager;
    SessionManagerPtr   session_manager;

    // Other items
    SceneContextPtr     scene_context;
};
