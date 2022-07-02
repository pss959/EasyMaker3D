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
    panel_ = mgr.GetTypedPanel<ToolPanel>("ToolPanel");
    InitPanel();

    // Attach the PanelChanged callback to Panel interaction.
    panel_->GetInteraction().AddObserver(
        this, [&](const std::string &key, ToolPanel::InteractionType type){
            PanelChanged(key, type);
        });

    // Set up the Board.
    mgr.PushPanel(panel_, nullptr);
    mgr.ShowBoard(context.board, true);

    // Position the Board above the attached Model. Put the bottom center of
    // the board just above the top center of the Model. This assumes the Board
    // is already in Stage coordinates.
    const float board_height = context.board->GetBounds().GetSize()[1];
    context.board->SetTranslation(
        GetPositionAboveModel(.5f * board_height + 2));
}

void PanelTool::Detach() {
    panel_->GetInteraction().RemoveObserver(this);

    GetContext().board_manager->ClosePanel("Done");

    panel_.reset();
}
