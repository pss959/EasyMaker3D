#include "Tools/PanelTool.h"

#include "Items/Board.h"
#include "Managers/BoardManager.h"

void PanelTool::UpdateGripInfo(GripInfo &info) {
    /// \todo (VR) Grip - Or does the Board handle it?
}

void PanelTool::Attach() {
    ASSERT(! panel_);
    const auto &context = GetContext();

    // Let the derived PanelTool class set up from the ToolPanel.
    auto &mgr = *context.board_manager;
    panel_ = mgr.GetTypedPanel<ToolPanel>(GetPanelName());
    InitPanel();

    // Attach the PanelChanged callback to Panel interaction.
    panel_->GetInteraction().AddObserver(
        this, [&](const std::string &key, ToolPanel::InteractionType type){
            PanelChanged(key, type);
        });

    // Set up the Board.
    context.board->SetPanel(panel_);
    mgr.ShowBoard(context.board, true);

    // If the Board is not in touch range, position it above and in front of
    // the attached Model. Note that GetPositionAboveModel() operates in stage
    // coordinates, but the ToolBoard needs to be positioned in world
    // coordinates.
    if (! context.board->IsInTouchPosition()) {
        const float board_height = context.board->GetBounds().GetSize()[1];
        const float kBoardZOffset = 1;
        context.board->SetTranslation(
            ToWorld(GetPositionAboveModel(0)) +
            Vector3f(0, .5f * board_height + 2, kBoardZOffset));
    }
}

void PanelTool::Detach() {
    // Detach from the Panel.
    panel_->GetInteraction().RemoveObserver(this);
    panel_.reset();

    // Close the Panel; this should also hide the Board.
    const auto &context = GetContext();
    context.board_manager->ClosePanel("Done");
    ASSERT(! context.board->IsShown());
}
