#include "Targets/PointTarget.h"

void PointTarget::AddFields() {
    AddField(position_);
    AddField(direction_);
    AddField(radius_);
    AddField(start_angle_);
    AddField(arc_angle_);
    Parser::Object::AddFields();
}

void PointTarget::CreationDone() {
    Parser::Object::CreationDone();

    // Make sure the direction is normalized.
    direction_ = ion::math::Normalized(direction_.GetValue());
}
