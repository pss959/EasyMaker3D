#include "SG/VRCamera.h"

namespace SG {

void VRCamera::AddFields() {
    AddField(base_position_);
    Camera::AddFields();
}

Point3f VRCamera::GetPosition() const {
    Point3f pos = GetBasePosition();
    pos[1] += GetHeight();
    return pos;
}

}  // namespace SG
