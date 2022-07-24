#include "SG/WindowCamera.h"

#include "Math/Linear.h"

namespace SG {

void WindowCamera::AddFields() {
    AddField(position_.Init("position", Point3f(0, 0, -10)));
    AddField(orientation_.Init("orientation"));
    AddField(fov_.Init("fov", Anglef::FromDegrees(60)));
    AddField(near_.Init("near", .01f));
    AddField(far_.Init("far", 20));
    AddField(controller_offset_.Init("controller_offset", Vector3f(0, 0, -10)));

    Camera::AddFields();
}

}  // namespace SG
