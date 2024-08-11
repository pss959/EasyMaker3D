//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Place/EdgeTarget.h"

#include <ion/math/vectorutils.h>

void EdgeTarget::AddFields() {
    AddField(position0_.Init("position0", Point3f::Zero()));
    AddField(position1_.Init("position1", Point3f(0, 4, 0)));

    Parser::Object::AddFields();
}

Vector3f EdgeTarget::GetDirection() const {
    return ion::math::Normalized(GetPosition1() - GetPosition0());
}

float EdgeTarget::GetLength() const {
    return ion::math::Distance(GetPosition0(), GetPosition1());
}
