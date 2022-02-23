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
    // the board just above the top of the Model, but leave Z as 0. This
    // assumes the Board is already in Stage coordinates.
    const Point3f model_top = GetStageCoordConv().ObjectToRoot(
        GetModelAttachedTo()->GetBounds().GetFaceCenter(Bounds::Face::kTop));
    const Vector3f board_size = context.board->GetBounds().GetSize();
    const Vector3f pos(model_top[0], model_top[1] + .5f * board_size[1] + 2, 0);
    context.board->SetTranslation(pos);
}

void PanelTool::Detach() {
    panel_->GetInteraction().RemoveObserver(this);

    const auto &context = GetContext();
    context.panel_manager->ClosePanel("Done");

    // Go back to using the default Board.
    context.panel_manager->SetCurrentBoard(BoardPtr());

    panel_.reset();
}
