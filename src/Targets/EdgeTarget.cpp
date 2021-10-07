#include "Targets/EdgeTarget.h"

#include <ion/math/vectorutils.h>

void EdgeTarget::AddFields() {
    AddField(position0_);
    AddField(position1_);
    Parser::Object::AddFields();
}

Vector3f EdgeTarget::GetDirection() const {
    return ion::math::Normalized(GetPosition1() - GetPosition0());
}

float EdgeTarget::GetLength() const {
    return ion::math::Distance(GetPosition0(), GetPosition1());
}
