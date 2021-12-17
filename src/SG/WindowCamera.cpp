#include "SG/WindowCamera.h"

#include "Math/Linear.h"

namespace SG {

void WindowCamera::AddFields() {
    AddField(position_);
    AddField(orientation_);
    AddField(fov_);
    AddField(near_);
    AddField(far_);
    Camera::AddFields();
}

}  // namespace SG
