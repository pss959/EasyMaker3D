#include "Tools/ColorTool.h"

#include "Managers/ColorManager.h"
#include "Managers/CommandManager.h"
#include "Math/Types.h"
#include "SG/Search.h"
#include "Util/Assert.h"

ColorTool::ColorTool() {
}

void ColorTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate()) {
        widget_ = SG::FindTypedNodeUnderNode<GenericWidget>(*this, "Widget");
        marker_ = SG::FindNodeUnderNode(*this, "Marker");
        disc_   = SG::FindNodeUnderNode(*this, "Disc");

        widget_->GetDragged().AddObserver(
            this, [&](const DragInfo *info, bool is_start){
                Dragged_(info, is_start); });
    }
}

void ColorTool::UpdateGripInfo(GripInfo &info) {
    // XXXX
}

void ColorTool::Attach() {
    ASSERT(widget_);
    UpdateColor_();

    // Position the ColorTool above the center of the attached Model.
    const float height = GetScaledBounds().GetSize()[1];
    SetTranslation(GetPositionAboveModel(.5f * height + 2));
}

void ColorTool::Detach() {
    // Nothing to do here.
}

void ColorTool::Dragged_(const DragInfo *info, bool is_start) {
    // Note that is_start is true for the start of a drag and info is null for
    // the end of a drag.
    if (is_start) {
        ASSERT(! command_);
        command_ = CreateCommand<ChangeColorCommand>("ChangeColorCommand");
        command_->SetFromSelection(GetSelection());
    }
    else if (info) {
        // Middle of the drag: simulate execution of the command to update all
        // the Models.
        ASSERT(command_);
        Color color = Color::White();  // XXXX
        command_->SetNewColor(color);
        GetContext().command_manager->SimulateDo(command_);
    }
    else {
        // End of the drag.
        ASSERT(command_);
        GetContext().command_manager->AddAndDo(command_);
        command_.reset();
    }
    UpdateColor_();
}

void ColorTool::UpdateColor_() {
    ASSERT(GetModelAttachedTo());
    const Color &color = GetModelAttachedTo()->GetColor();

    // Move the marker to the correct spot.
    // XXXX

    // Change the color of the central disc.
    disc_->SetBaseColor(color);
}
