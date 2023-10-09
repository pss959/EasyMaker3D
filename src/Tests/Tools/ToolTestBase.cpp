#include "Tests/Tools/ToolTestBase.h"

#include "Managers/BoardManager.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/PanelManager.h"
#include "Managers/SettingsManager.h"
#include "Managers/TargetManager.h"
#include "Models/RootModel.h"
#include "Panels/Board.h"
#include "Place/PrecisionStore.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/String.h"
#include "Widgets/EdgeTargetWidget.h"
#include "Widgets/PointTargetWidget.h"

void ToolTestBase::AddDummyCommandFunction(const Str &name) {
    ASSERT(context);
    context->command_manager->RegisterFunction(
        name, [](Command &, Command::Op){});
}

Str ToolTestBase::GetContentsString_(const Str &name) {
    // Have to set up a Board and target Widgets in addition to the named Tool.
    const Str str = R"(
  children: [
    Node {
      TEMPLATES: [
        <"nodes/templates/Frame.emd">,
        <"nodes/templates/Board.emd">
      ],
      children: [
        <"nodes/Tools/<NAME>.emd">,         # Has to be first.
        <"nodes/templates/RadialMenu.emd">, # Required for RadialMenuPanel.
        <"nodes/Panels.emd">,
        <"nodes/ModelRoot.emd">,
        <"nodes/Widgets/PointTargetWidget.emd">,
        <"nodes/Widgets/EdgeTargetWidget.emd">,
        CLONE "T_Board" "TestBoard" {},
      ]
    }
  ]
)";

    return Util::ReplaceString(str, "<NAME>", name);
}

void ToolTestBase::SetUpTool_(const ToolPtr &tool) {
    ASSERT(tool);
    context.reset(new Tool::Context);

    auto &scene = *GetScene();

    PanelManagerPtr pm(new PanelManager);
    context->board_manager.reset(new BoardManager(pm));
    context->command_manager.reset(new CommandManager);
    context->feedback_manager.reset(new FeedbackManager);
    context->precision_store.reset(new PrecisionStore);
    context->settings_manager.reset(new SettingsManager);
    context->target_manager.reset(new TargetManager(context->command_manager));
    context->root_model = SG::FindTypedNodeInScene<RootModel>(scene,
                                                              "ModelRoot");
    context->board = SG::FindTypedNodeInScene<Board>(scene, "TestBoard");
    context->path_to_parent_node = SG::NodePath(tool);
    context->camera_position.Set(0, 0, 10);

    auto ptw = SG::FindTypedNodeInScene<PointTargetWidget>(
        scene, "PointTargetWidget");
    auto etw = SG::FindTypedNodeInScene<EdgeTargetWidget>(
        scene, "EdgeTargetWidget");
    context->target_manager->InitTargets(ptw, etw);

    tool->SetContext(context);
}
