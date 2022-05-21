#include "Tools/PanelTool.h"

#include "Items/Board.h"
#include "Managers/PanelManager.h"

void PanelTool::UpdateGripInfo(GripInfo &info) {
    /// \todo (VR) Grip - Or does the Board handle it?
}

void PanelTool::Attach() {
    ASSERT(! panel_);
    const auto &context = GetContext();

    // Use the tool Board for the Panel.
    context.panel_manager->SetCurrentBoard(context.board);

    auto init_panel = [&](const PanelPtr &panel){
        panel_ = Util::CastToDerived<ToolPanel>(panel);
        ASSERT(panel_);
        // Let the derived PanelTool class set up from the Panel.
        InitPanel();
        // Attach the PanelChanged callback to Panel interaction.
        panel_->GetInteraction().AddObserver(
            this, [&](const std::string &key, ToolPanel::InteractionType type){
                PanelChanged(key, type);
            });
    };

    // Open the Panel, save it and call the initialization function.
    context.panel_manager->InitAndOpenPanel(GetPanelName(), init_panel);

    // Position the Board above the attached Model. Put the bottom center of
    // the board just above the top center of the Model. This assumes the Board
    // is already in Stage coordinates.
    const float board_height = context.board->GetBounds().GetSize()[1];
    context.board->SetTranslation(
        GetPositionAboveModel(.5f * board_height + 2));
}

void PanelTool::Detach() {
    panel_->GetInteraction().RemoveObserver(this);

    const auto &context = GetContext();
    context.panel_manager->ClosePanel("Done");

    // Go back to using the default Board.
    context.panel_manager->SetCurrentBoard(BoardPtr());

    panel_.reset();
}
