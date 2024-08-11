//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/VRCamera.h"

namespace SG {

void VRCamera::AddFields() {
    AddField(base_position_.Init("base_position", Point3f::Zero()));

    Camera::AddFields();
}

Point3f VRCamera::GetCurrentPosition() const {
    Point3f pos = GetBasePosition();
    pos[1] += GetHeight();
    return pos;
}

}  // namespace SG
