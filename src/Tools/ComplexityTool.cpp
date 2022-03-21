#include "Tools/ComplexityTool.h"

#include "Managers/CommandManager.h"
#include "SG/Search.h"
#include "Util/Assert.h"

ComplexityTool::ComplexityTool() {
}

void ComplexityTool::CreationDone() {
    Tool::CreationDone();

    if (! IsTemplate())
        FindParts_();
}

void ComplexityTool::UpdateGripInfo(GripInfo &info) {
    // XXXX
}

void ComplexityTool::Attach() {
    ASSERT(GetModelAttachedTo());

    // Position the ComplexityTool above the center of the attached Model.
    const float height = GetScaledBounds().GetSize()[1];
    SetTranslation(GetPositionAboveModel(.5f * height + 2));
}

void ComplexityTool::Detach() {
    // Nothing to do here.
}

void ComplexityTool::FindParts_() {
    ASSERT(! slider_);
    slider_ = SG::FindTypedNodeUnderNode<Slider1DWidget>(*this, "Slider");

    slider_->GetActivation().AddObserver(
        this, [&](Widget &, bool is_act){ SliderActivated_(is_act); });
    slider_->GetValueChanged().AddObserver(
        this, [&](Widget &, const float &val){ SliderChanged_(val); });
    slider_->GetValueChanged().EnableObserver(this, false);
}

void ComplexityTool::SliderActivated_(bool is_act) {
    if (is_act) {
        slider_->GetValueChanged().EnableObserver(this, true);
    }
    else if (command_) {
        slider_->GetValueChanged().EnableObserver(this, false);
        GetContext().command_manager->AddAndDo(command_);
        command_.reset();
    }
}

void ComplexityTool::SliderChanged_(float value) {
    if (! command_) {
        command_ =
            CreateCommand<ChangeComplexityCommand>("ChangeComplexityCommand");
        command_->SetFromSelection(GetSelection());
    }
    command_->SetNewComplexity(value);
    GetContext().command_manager->SimulateDo(command_);
}
