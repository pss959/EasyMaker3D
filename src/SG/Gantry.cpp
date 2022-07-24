#include "SG/Gantry.h"

namespace SG {

void Gantry::AddFields() {
    AddField(cameras_.Init("cameras"));

    Object::AddFields();
}

void Gantry::SetHeight(float height) {
    height_ = height;

    // Update all of the cameras.
    for (auto &cam: GetCameras())
        cam->SetHeight(height);
}

}  // namespace SG
