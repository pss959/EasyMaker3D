#include "Tools/PanelTool.h"

#include "Managers/BoardManager.h"
#include "Panels/Board.h"
#include "Util/Tuning.h"

void PanelTool::Attach() {
    ASSERT(! panel_);
    const auto &context = GetContext();

    // Set the panel_ member for InitPanel().
    auto &mgr = *context.board_manager;
    panel_ = mgr.GetTypedPanel<ToolPanel>(GetPanelName());

    // Let the derived PanelTool class set up from the ToolPanel.
    InitPanel();

    // Attach the PanelChanged callback to Panel interaction.
    panel_->GetInteraction().AddObserver(
        this, [&](const Str &key, ToolPanel::InteractionType type){
            if (type == ToolPanel::InteractionType::kDragStart)
                GetDragStarted().Notify(*this);
            PanelChanged(key, type);
            if (type == ToolPanel::InteractionType::kDragEnd)
                GetDragEnded().Notify(*this);
        });

    // Set up and position the Board.
    context.board->SetPanel(panel_);
    mgr.ShowBoard(context.board, true);
    UpdateBoardPosition_();
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

void PanelTool::ReattachToSelection() {
    // The Tool class defines this to call Detach() and Attach(). For a
    // PanelTool, that would mean closing and reopening the Panel, which is
    // overkill. Instead, just reinitialize the Panel and update the Board's
    // position.
    InitPanel();
    UpdateBoardPosition_();
}

void PanelTool::UpdateBoardPosition_() {
    // Position the Board above and in front of the attached Model. Note that
    // GetPositionAboveModel() operates in stage coordinates, but the ToolBoard
    // needs to be positioned in world coordinates. This all has no effect when
    // in VR and the Board is set up for touch interaction.
    const auto &context = GetContext();
    const float board_height = context.board->GetBounds().GetSize()[1];
    context.board->SetPosition(
        ToWorld(GetPositionAboveModel(0)) +
        Vector3f(0, .5f * board_height + 2, TK::kToolBoardZOffset));
}
