#include "SG/Camera.h"

#include "SG/SpecBuilder.h"
#include "Util/String.h"

namespace SG {

Camera::FOV::FOV() {
    right = up   = Anglef::FromDegrees(30.f);
    left  = down = -right;
}

Camera::FOV::FOV(const Anglef &vfov, float aspect) {
    up    = .5f * vfov;
    down  = -up;
    right = aspect * up;
    left  = -right;
}

Camera::Camera() : position_(0.f, 0.f, -10.f), near_(.01f),  far_(20.f) {}

std::string Camera::ToString() const {
    return ("Cam [pos="     + Util::ToString(position_) +
                " or="      + Util::ToString(orientation_) +
                " fov=(l:"  + Util::ToString(fov_.left) +
                " r:"       + Util::ToString(fov_.right) +
                " u:,"      + Util::ToString(fov_.up) +
                " d:,"      + Util::ToString(fov_.down) +
                ") nr="     + Util::ToString(near_) +
                " fr="      + Util::ToString(far_) +
                "]");
}

void Camera::Finalize() {
    fov_ = FOV(fov_in_, 1.f);  // XXXX Real aspect ratio?
}

NParser::ObjectSpec Camera::GetObjectSpec() {
    SG::SpecBuilder<Camera> builder;
    builder.AddVector3f("position",     &Camera::position_);
    builder.AddRotationf("orientation", &Camera::orientation_);
    builder.AddAnglef("fov",            &Camera::fov_in_);
    builder.AddFloat("near",            &Camera::near_);
    builder.AddFloat("far",             &Camera::far_);
    return NParser::ObjectSpec{
        "Camera", []{ return new Camera; }, builder.GetSpecs() };
}

}  // namespace SG
