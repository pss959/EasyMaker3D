//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Place/PointTarget.h"

void PointTarget::AddFields() {
    AddField(position_.Init("position", Point3f::Zero()));
    AddField(direction_.Init("direction", Vector3f::AxisY()));
    AddField(radius_.Init("radius", 1));
    AddField(arc_.Init("arc"));

    Parser::Object::AddFields();
}

void PointTarget::CreationDone() {
    Parser::Object::CreationDone();

    // Make sure the direction is normalized.
    direction_ = ion::math::Normalized(direction_.GetValue());
}
