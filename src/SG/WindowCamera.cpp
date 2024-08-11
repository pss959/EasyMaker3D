//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/WindowCamera.h"

#include "Math/Frustum.h"
#include "Math/Linear.h"

namespace SG {

void WindowCamera::AddFields() {
    AddField(position_.Init("position", Point3f(0, 0, -10)));
    AddField(orientation_.Init("orientation"));
    AddField(fov_.Init("fov", Anglef::FromDegrees(60)));
    AddField(near_.Init("near", .01f));
    AddField(far_.Init("far", 20));

    Camera::AddFields();
}

void WindowCamera::BuildFrustum(const Vector2ui window_size,
                                Frustum &frustum) const {
    frustum.viewport     = Viewport::BuildWithSize(Point2ui(0, 0), window_size);
    frustum.position     = GetPosition();
    frustum.position[1] += GetHeight();
    frustum.orientation  = GetOrientation();
    frustum.pnear        = GetNear();
    frustum.pfar         = GetFar();

    // Create a symmetric FOV.
    frustum.SetSymmetricFOV(GetFOV(), GetAspectRatio(frustum.viewport));
}

}  // namespace SG
