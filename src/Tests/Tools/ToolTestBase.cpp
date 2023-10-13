#include "Tests/Tools/ToolTestBase.h"

#include "Commands/CommandList.h"
#include "Feedback/FindFeedback.h"
#include "Managers/BoardManager.h"
#include "Managers/CommandManager.h"
#include "Managers/FeedbackManager.h"
#include "Managers/NameManager.h"
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

void ToolTestBase::TearDown() {
    // Make sure the Board has at most one Panel or assertions will fail.
    Str result;
    while (context->board->GetPanelCount() > 1U)
        context->board->PopPanel(result);
}

void ToolTestBase::AddCommandFunction(
    const Str &name, const std::function<void(const Command &)> &func) {
    ASSERT(context);
    ASSERT(func);
    auto exec_func = [&, func](Command &cmd, Command::Op op){
        if (op == Command::Op::kDo)
            func(cmd);
    };
    context->command_manager->RegisterFunction(name, exec_func);
}

void ToolTestBase::AddDummyCommandFunction(const Str &name) {
    ASSERT(context);
    context->command_manager->RegisterFunction(
        name, [](Command &, Command::Op){});
}

void ToolTestBase::CheckNoCommands() {
    const auto &cl = *context->command_manager->GetCommandList();
    EXPECT_EQ(0U, cl.GetCommandCount());
}

void ToolTestBase::SetPointTarget(const Point3f &pos, const Vector3f &dir) {
    auto pt = CreateObject<PointTarget>();
    pt->SetPosition(pos);
    pt->SetDirection(dir);
    context->target_manager->SetPointTarget(*pt);
    context->target_manager->SetPointTargetVisible(true);
}

void ToolTestBase::SetEdgeTargetLength(float length) {
    auto et = CreateObject<EdgeTarget>();
    et->SetPositions(Point3f(0, 0, 0), Point3f(length, 0, 0));
    context->target_manager->SetEdgeTarget(*et);
    context->target_manager->SetEdgeTargetVisible(true);
}

Str ToolTestBase::GetContentsString_() {
    // Have to set up a Board and target Widgets in addition to the Tools.
    return R"(
  children: [
    Node {
      TEMPLATES: [
        <"nodes/templates/Frame.emd">,
        <"nodes/templates/Board.emd">
        <"nodes/templates/Border.emd">,
        <"nodes/templates/PaneBackground.emd">,
        <"nodes/templates/RadialMenu.emd">,
      ],
      children: [
        <"nodes/DimensionColors.emd">,
        <"nodes/Tools.emd">,             # Has to precede Panels.
        <"nodes/Panels.emd">,
        <"nodes/ModelRoot.emd">,
        <"nodes/Widgets/PointTargetWidget.emd">,
        <"nodes/Widgets/EdgeTargetWidget.emd">,
        CLONE "T_Board" "TestBoard" {},
        <"nodes/Feedback.emd">,
      ]
    }
  ]
)";
}

void ToolTestBase::SetUpTool_(const ToolPtr &tool) {
    ASSERT(tool);

    auto       &scene = *GetScene();
    const auto &root  = scene.GetRootNode();

    // Create a parent Node for the Tool. This allows for coordinate conversion
    // testing.
    tool_parent = CreateObject<SG::Node>("ToolParent");
    tool_parent->AddChild(tool);

    // Set up the Tool::Context.
    context.reset(new Tool::Context);

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
    context->path_to_parent_node = SG::NodePath(tool_parent);
    context->camera_position.Set(0, 0, 10);

    // Set up the FeedbackManager.
    context->feedback_manager->SetParentNodes(root, root);
    context->feedback_manager->SetSceneBoundsFunc(
        [&](){ return context->root_model->GetBounds(); });
    context->feedback_manager->SetPathToStage(SG::NodePath(root));
    for (auto &fb: FindFeedback(*root))
        context->feedback_manager->AddOriginal<Feedback>(fb);

    // Set up the TargetManager.
    auto ptw = SG::FindTypedNodeInScene<PointTargetWidget>(
        scene, "PointTargetWidget");
    auto etw = SG::FindTypedNodeInScene<EdgeTargetWidget>(
        scene, "EdgeTargetWidget");
    context->target_manager->InitTargets(ptw, etw);

    // Set up the PanelManager including a Panel::Context that contains the
    // bare minimum for tests.
    Panel::ContextPtr pc(new Panel::Context);
    pc->board_agent = context->board_manager;
    pc->name_agent.reset(new NameManager);
    pm->FindAllPanels(scene, pc);

    tool->SetContext(context);
}

const Command * ToolTestBase::CheckNCommands_(size_t count) {
    const auto &cl = *context->command_manager->GetCommandList();
    EXPECT_EQ(count, cl.GetCommandCount());
    return cl.GetCommand(count - 1).get();
}
