#include "SG/Camera.h"

#include "Math/Types.h"
#include "Util/String.h"

namespace SG {

void Camera::AddFields() {
    AddField(position_);
    AddField(orientation_);
    AddField(fov_);
    AddField(near_);
    AddField(far_);
}

Frustum Camera::BuildFrustum(float aspect) const {
    Frustum frustum;
    frustum.position    = position_;
    frustum.orientation = orientation_;

    // Create a symmetric FOV.
    frustum.SetSymmetricFOV(fov_, aspect);

    frustum.near = near_;
    frustum.far  = far_;

    return frustum;
}

}  // namespace SG
