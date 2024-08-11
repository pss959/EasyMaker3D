//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tools/FloatingTool.h"

void FloatingTool::Update() {
    // Rotate to face the camera if necessary.
    const Vector3f z = Vector3f::AxisZ();
    const Vector3f world_z = ToWorld(z);
    SetRotation(Rotationf::RotateInto(world_z, z));
}
