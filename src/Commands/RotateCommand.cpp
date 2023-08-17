#include "Commands/RotateCommand.h"

void RotateCommand::AddFields() {
    AddField(rotation_.Init("rotation"));
    AddField(is_in_place_.Init("is_in_place", false));
    AddField(is_axis_aligned_.Init("is_axis_aligned", false));

    MultiModelCommand::AddFields();
}

Str RotateCommand::GetDescription() const {
    return "Rotated " + GetModelsDesc(GetModelNames()) +
        (IsInPlace() ? " (in place)" : "");
}
