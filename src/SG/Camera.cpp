#include "SG/Camera.h"

#include "Frustum.h"
#include "SG/Math.h"
#include "SG/SpecBuilder.h"
#include "Util/String.h"

namespace SG {

Camera::Camera() : position_(0.f, 0.f, -10.f),
                   fov_(Anglef::FromDegrees(60)),
                   near_(.01f),
                   far_(20.f) {}

Frustum Camera::BuildFrustum(float aspect) const {
    Frustum frustum;
    frustum.position    = position_;
    frustum.orientation = orientation_;

    // Create a symmetric FOV.
    frustum.fov_right   = .5f * aspect * fov_;
    frustum.fov_up      = .5f * fov_;
    frustum.fov_left    = -frustum.fov_right;
    frustum.fov_down    = -frustum.fov_up;

    frustum.near        = near_;
    frustum.far         = far_;

    return frustum;
}

NParser::ObjectSpec Camera::GetObjectSpec() {
    SG::SpecBuilder<Camera> builder;
    builder.AddVector3f("position",     &Camera::position_);
    builder.AddRotationf("orientation", &Camera::orientation_);
    builder.AddAnglef("fov",            &Camera::fov_);
    builder.AddFloat("near",            &Camera::near_);
    builder.AddFloat("far",             &Camera::far_);
    return NParser::ObjectSpec{
        "Camera", false, []{ return new Camera; }, builder.GetSpecs() };
}

}  // namespace SG
