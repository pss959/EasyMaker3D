#include "PanelTool.h"

#include "Managers/PanelManager.h"

void PanelTool::UpdateGripInfo(GripInfo &info) {
    // XXXX Do something?  Or does the Board handle it?
}

void PanelTool::Attach() {
    ASSERT(! panel_);
    const auto &context = GetContext();

    // Use the tool Board for the Panel.
    context.panel_manager->SetCurrentBoard(context.board);

    // Open the Panel, save it and let the derived class initialize it.
    auto init_panel = [&](const PanelPtr &panel){
        panel_ = panel;
        InitPanel();
    };
    context.panel_manager->InitAndOpenPanel(GetPanelName(), init_panel);
}

void PanelTool::Detach() {
    const auto &context = GetContext();
    context.panel_manager->ClosePanel("Done");

    // Go back to using the default Board.
    context.panel_manager->SetCurrentBoard(BoardPtr());
}

void PanelTool::PanelChanged(const std::string &key,
                             ToolPanel::InteractionType type) {
    if      (type == ToolPanel::InteractionType::kDragStart)
        is_dragging_ = true;
    else if (type == ToolPanel::InteractionType::kDragEnd)
        is_dragging_ = false;
}
