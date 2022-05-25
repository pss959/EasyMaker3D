#include "Targets/PointTarget.h"

void PointTarget::AddFields() {
    AddField(position_);
    AddField(direction_);
    AddField(radius_);
    AddField(arc_);
    Parser::Object::AddFields();
}

void PointTarget::CreationDone() {
    Parser::Object::CreationDone();

    // Make sure the direction is normalized.
    direction_ = ion::math::Normalized(direction_.GetValue());
}
