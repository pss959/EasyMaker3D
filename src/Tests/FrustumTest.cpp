#include <ion/math/vectorutils.h>

#include "Frustum.h"
#include "Testing.h"
#include "SG/Math.h"

class FrustumTest : public TestBase {
};

TEST_F(FrustumTest, SetSymmetricFOV) {
    Frustum f;
    f.SetSymmetricFOV(SG::Anglef::FromDegrees(90.f), 2.f);

    SG::Anglef a = SG::Anglef::FromDegrees(45.f);
    EXPECT_EQ(-2 * a, f.fov_left);
    EXPECT_EQ( 2 * a, f.fov_right);
    EXPECT_EQ(-a,     f.fov_down);
    EXPECT_EQ( a,     f.fov_up);
}

TEST_F(FrustumTest, BuildRay) {
    // Start with a default frustum.
    Frustum f;
    SG::Ray ray;

    // Ray in the center.
    ray = f.BuildRay(SG::Point2f(.5f, .5f));
    EXPECT_PRED2(PointsClose,  SG::Point3f(0, 0, 10 - f.near), ray.origin);
    EXPECT_PRED2(VectorsClose, SG::Vector3f(0, 0, -1), ray.direction);

    // Set up with easier math.
    f.SetSymmetricFOV(SG::Anglef::FromDegrees(90.f), 1.f);
    f.near = 2.f;

    // Ray in the center.
    ray = f.BuildRay(SG::Point2f(.5f, .5f));
    EXPECT_PRED2(PointsClose,  SG::Point3f(0, 0, 8),   ray.origin);
    EXPECT_PRED2(VectorsClose, SG::Vector3f(0, 0, -1), ray.direction);

    // Ray in the lower-left corner
    ray = f.BuildRay(SG::Point2f(0, 0));
    EXPECT_PRED2(PointsClose, SG::Point3f(-2, -2, 8), ray.origin);
    // Ray should pass from position through origin.
    EXPECT_PRED2(VectorsClose,
                 ion::math::Normalized(ray.origin - f.position), ray.direction);

    // Ray in the lower-right corner
    ray = f.BuildRay(SG::Point2f(1, 0));
    EXPECT_PRED2(PointsClose, SG::Point3f(2, -2, 8), ray.origin);
    EXPECT_PRED2(VectorsClose,
                 ion::math::Normalized(ray.origin - f.position), ray.direction);

    // Ray in the upper-left corner
    ray = f.BuildRay(SG::Point2f(0, 1));
    EXPECT_PRED2(PointsClose, SG::Point3f(-2, 2, 8), ray.origin);
    EXPECT_PRED2(VectorsClose,
                 ion::math::Normalized(ray.origin - f.position), ray.direction);

    // Ray in the upper-right corner
    ray = f.BuildRay(SG::Point2f(1, 1));
    EXPECT_PRED2(PointsClose, SG::Point3f(2, 2, 8), ray.origin);
    EXPECT_PRED2(VectorsClose,
                 ion::math::Normalized(ray.origin - f.position), ray.direction);
}

TEST_F(FrustumTest, BuildRayTransformed) {
    Frustum f;
    f.near = 1.f;
    f.position.Set(0, 10, 40);
    SG::Ray ray = f.BuildRay(SG::Point2f(.5f, .5f));
    EXPECT_PRED2(PointsClose,  SG::Point3f(0, 10, 39),   ray.origin);
    EXPECT_PRED2(VectorsClose, SG::Vector3f(0, 0, -1), ray.direction);

    // Rotate 90 degrees around the Y axis.
    f.orientation = SG::Rotationf::FromAxisAndAngle(SG::Vector3f::AxisY(),
                                                    SG::Anglef::FromDegrees(90));
    ray = f.BuildRay(SG::Point2f(.5f, .5f));
    EXPECT_PRED2(PointsClose,  SG::Point3f(-1, 10, 40), ray.origin);
    EXPECT_PRED2(VectorsClose, SG::Vector3f(-1, 0, 0),  ray.direction);
}
