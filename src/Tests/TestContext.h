#pragma once

#include "Base/Memory.h"

DECL_SHARED_PTR(ActionProcessor);
DECL_SHARED_PTR(CommandManager);
DECL_SHARED_PTR(PanelManager);
DECL_SHARED_PTR(SceneContext);
DECL_SHARED_PTR(SelectionManager);
DECL_SHARED_PTR(SessionManager);
DECL_SHARED_PTR(SettingsManager);

/// This struct defines a context that can be provided to unit tests. More can
/// be added to it as necessary. The Application class sets up an instance on
/// demand.
struct TestContext {
    // Managers
    ActionProcessorPtr  action_processor;
    CommandManagerPtr   command_manager;
    PanelManagerPtr     panel_manager;
    SelectionManagerPtr selection_manager;
    SessionManagerPtr   session_manager;
    SettingsManagerPtr  settings_manager;

    // Other items
    SceneContextPtr     scene_context;
};
