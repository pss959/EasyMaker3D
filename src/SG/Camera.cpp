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

void Camera::SetFieldParsed(const Parser::Field &field) {
    if (field.GetName() == "position")
        starting_position_ = position_;
}

void Camera::SetHeight(float height) {
    position_ = starting_position_ + Vector3f(0, height, 0);
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
