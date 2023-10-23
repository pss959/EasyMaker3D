#include "Tests/Tools/TestToolContext.h"

#include "Feedback/FindFeedback.h"
#include "Managers/BoardManager.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/NameManager.h"
#include "Managers/PanelManager.h"
#include "Managers/SettingsManager.h"
#include "Managers/TargetManager.h"
#include "Models/RootModel.h"
#include "Place/PrecisionStore.h"
#include "SG/Scene.h"
#include "SG/Search.h"

TestToolContext::TestToolContext(const SG::Scene &scene) {
    const auto &root = scene.GetRootNode();

    // Find important nodes in the scene.
    root_model = SG::FindTypedNodeInScene<RootModel>(scene, "ModelRoot");

    // Create managers.
    NameManagerPtr  nm(new NameManager);
    PanelManagerPtr pm(new PanelManager);
    board_manager.reset(new BoardManager(pm));
    command_manager.reset(new CommandManager);
    feedback_manager.reset(new FeedbackManager);
    precision_store.reset(new PrecisionStore);
    settings_manager.reset(new SettingsManager);
    target_manager.reset(new TargetManager(command_manager));

    // Set up the FeedbackManager.
    feedback_manager->SetParentNodes(root, root);
    feedback_manager->SetSceneBoundsFunc([&](){
        return root_model->GetBounds(); });
    feedback_manager->SetPathToStage(SG::NodePath(root));
    for (auto &fb: FindFeedback(*root))
        feedback_manager->AddOriginal<Feedback>(fb);

    // Set up the PanelManager including a Panel::Context that contains the
    // bare minimum for tests.
    Panel::ContextPtr pc(new Panel::Context);
    pc->board_agent = board_manager;
    pc->name_agent  = nm;
    pm->FindAllPanels(scene, pc);

    path_to_parent_node = SG::NodePath(root);
    camera_position.Set(0, 0, 10);
}

