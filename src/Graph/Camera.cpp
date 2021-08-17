#include "Graph/Camera.h"

#include "Util/String.h"

namespace Graph {

Camera::FOV::FOV() {
    right = up   = ion::math::Anglef::FromDegrees(30.f);
    left  = down = -right;
}

Camera::FOV::FOV(const ion::math::Anglef &vfov, float aspect) {
    up    = .5f * vfov;
    down  = -up;
    right = aspect * up;
    left  = -right;
}

std::string Camera::ToString() const {
    return ("Cam [pos="     + Util::ToString(position) +
                " or="      + Util::ToString(orientation) +
                " fov=(l:"  + Util::ToString(fov.left) +
                " r:"       + Util::ToString(fov.right) +
                " u:,"      + Util::ToString(fov.up) +
                " d:,"      + Util::ToString(fov.down) +
                ") nr="     + Util::ToString(near) +
                " fr="      + Util::ToString(far) +
                "]");
}

}  // namespace Graph
