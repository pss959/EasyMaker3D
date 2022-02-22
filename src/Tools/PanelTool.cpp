#include "PanelTool.h"

#include "Managers/PanelManager.h"

void PanelTool::UpdateGripInfo(GripInfo &info) {
    // XXXX Do something?  Or does the Board handle it?
}

void PanelTool::ReattachToSelection() {
    if (! is_dragging_)
        SpecializedTool::ReattachToSelection();
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
}

void PanelTool::Detach() {
    panel_->GetInteraction().RemoveObserver(this);

    const auto &context = GetContext();
    context.panel_manager->ClosePanel("Done");

    // Go back to using the default Board.
    context.panel_manager->SetCurrentBoard(BoardPtr());

    panel_.reset();
}

void PanelTool::PanelChanged(const std::string &key,
                             ToolPanel::InteractionType type) {
    if      (type == ToolPanel::InteractionType::kDragStart)
        is_dragging_ = true;
    else if (type == ToolPanel::InteractionType::kDragEnd)
        is_dragging_ = false;
}
