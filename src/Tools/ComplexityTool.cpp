#include "Tools/ComplexityTool.h"

#include "Managers/CommandManager.h"
#include "SG/Search.h"
#include "Selection/Selection.h"
#include "Models/Model.h"
#include "Util/Assert.h"

ComplexityTool::ComplexityTool() {
}

void ComplexityTool::CreationDone() {
    FloatingTool::CreationDone();

    if (! IsTemplate())
        FindParts_();
}

void ComplexityTool::UpdateGripInfo(GripInfo &info) {
    info.guide_type   = GripGuideType::kBasic;
    info.widget       = slider_;
    info.target_point = ToWorld(slider_, Point3f::Zero());
}

bool ComplexityTool::CanAttach(const Selection &sel) const {
    return sel.GetPrimary().GetModel()->CanSetComplexity();
}

void ComplexityTool::Attach() {
    ASSERT(GetModelAttachedTo());

    // Position the ComplexityTool above the center of the attached Model.
    const float height = GetScaledBounds().GetSize()[1];
    SetTranslation(GetPositionAboveModel(.5f * height + 2, false));

    // Set the slider based on the current complexity.
    const float complexity = GetModelAttachedTo()->GetComplexity();
    slider_->SetValue(complexity);
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
        GetDragEnded().Notify(*this);
        GetContext().command_manager->AddAndDo(command_);
        command_.reset();
    }
}

void ComplexityTool::SliderChanged_(float value) {
    if (! command_) {
        command_ = CreateCommand<ChangeComplexityCommand>();
        command_->SetFromSelection(GetSelection());
        GetDragStarted().Notify(*this);
    }
    command_->SetNewComplexity(value);
    GetContext().command_manager->SimulateDo(command_);
}
