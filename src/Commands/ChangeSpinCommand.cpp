#include "Commands/ChangeSpinCommand.h"

#include "Math/Linear.h"
#include "Util/Assert.h"

void ChangeSpinCommand::AddFields() {
    AddField(center_.Init("center"));
    AddField(axis_.Init("axis"));
    AddField(angle_.Init("angle"));
    AddField(offset_.Init("offset"));
    MultiModelCommand::AddFields();
}

bool ChangeSpinCommand::IsValid(std::string &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (! IsValidVector(axis_)) {
        details = "zero-length spin axis vector";
        return false;
    }
    return true;
}

void ChangeSpinCommand::SetSpin(const Spin &spin) {
    ASSERT(IsValidVector(spin.axis));
    center_ = spin.center;
    axis_   = spin.axis;
    angle_  = spin.angle;
    offset_ = spin.offset;
}

Spin ChangeSpinCommand::GetSpin() const {
    Spin spin;
    spin.center = center_;
    spin.axis   = axis_;
    spin.angle  = angle_;
    spin.offset = offset_;
    return spin;
}
