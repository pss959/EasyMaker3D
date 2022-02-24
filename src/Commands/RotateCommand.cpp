#include "Commands/RotateCommand.h"

void RotateCommand::AddFields() {
    AddField(rotation_);
    AddField(is_in_place_);
    AddField(is_axis_aligned_);
    MultiModelCommand::AddFields();
}

std::string RotateCommand::GetDescription() const {
    return "Rotated " + GetModelsDesc(GetModelNames()) +
        (IsInPlace() ? " (in place)" : "");
}
