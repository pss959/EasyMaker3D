#include "SG/Camera.h"

#include "Math/Types.h"
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
    frustum.SetSymmetricFOV(fov_, aspect);

    frustum.near = near_;
    frustum.far  = far_;

    return frustum;
}

Parser::ObjectSpec Camera::GetObjectSpec() {
    SG::SpecBuilder<Camera> builder;
    builder.AddPoint3f("position",      &Camera::position_);
    builder.AddRotationf("orientation", &Camera::orientation_);
    builder.AddAnglef("fov",            &Camera::fov_);
    builder.AddFloat("near",            &Camera::near_);
    builder.AddFloat("far",             &Camera::far_);
    return Parser::ObjectSpec{
        "Camera", false, []{ return new Camera; }, builder.GetSpecs() };
}

}  // namespace SG
