#include <ion/math/angleutils.h>

#include "Math/Types.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class FrustumTest : public TestBase {};

TEST_F(FrustumTest, SetSymmetricFOV) {
    Frustum f;
    {
        // 2:1 aspect ratio.
        f.SetSymmetricFOV(Anglef::FromDegrees(90.f), 2.f);
        const Anglef a = Anglef::FromDegrees(45.f);
        EXPECT_EQ(-a, f.fov_left);
        EXPECT_EQ( a, f.fov_right);
        const Anglef half_vfov =
            ion::math::ArcTangent(ion::math::Tangent(a) / 2);
        EXPECT_EQ(-half_vfov, f.fov_down);
        EXPECT_EQ( half_vfov, f.fov_up);
        EXPECT_VECS_CLOSE(Vector3f(0, 0, -1), f.GetViewDirection());
    }
    {
        // 1:2 aspect ratio.
        f.SetSymmetricFOV(Anglef::FromDegrees(90.f), 0.5f);
        const Anglef a = Anglef::FromDegrees(26.5651f);
        EXPECT_CLOSE(-a.Degrees(), f.fov_left.Degrees());
        EXPECT_CLOSE( a.Degrees(), f.fov_right.Degrees());
        const Anglef half_vfov = Anglef::FromDegrees(45);
        EXPECT_EQ(-half_vfov, f.fov_down);
        EXPECT_EQ( half_vfov, f.fov_up);
        EXPECT_VECS_CLOSE(Vector3f(0, 0, -1), f.GetViewDirection());
    }
}

TEST_F(FrustumTest, SetFromTangents) {
    using ion::math::Tangent;
    Frustum f;
    f.SetFromTangents(Tangent(-45), Tangent(45), Tangent(-45), Tangent(45));
    const Anglef a = Anglef::FromDegrees(45.f);
    EXPECT_EQ(-a, f.fov_left);
    EXPECT_EQ( a, f.fov_right);
    EXPECT_EQ(-a, f.fov_down);
    EXPECT_EQ( a, f.fov_up);
    EXPECT_VECS_CLOSE(Vector3f(0, 0, -1), f.GetViewDirection());
}

TEST_F(FrustumTest, BuildRay) {
    // Start with a default frustum.
    Frustum f;
    Ray ray;

    // Ray in the center.
    ray = f.BuildRay(Point2f(.5f, .5f));
    EXPECT_PTS_CLOSE(Point3f(0, 0, 10 - f.pnear), ray.origin);
    EXPECT_VECS_CLOSE(Vector3f(0, 0, -1), ray.direction);

    // Set up with easier math.
    f.SetSymmetricFOV(Anglef::FromDegrees(90.f), 1.f);
    f.pnear = 2.f;

    // Ray in the center.
    ray = f.BuildRay(Point2f(.5f, .5f));
    EXPECT_PTS_CLOSE(Point3f(0, 0, 8),   ray.origin);
    EXPECT_VECS_CLOSE(Vector3f(0, 0, -1), ray.direction);

    // Ray in the lower-left corner
    ray = f.BuildRay(Point2f(0, 0));
    EXPECT_PTS_CLOSE(Point3f(-2, -2, 8), ray.origin);
    // Ray should pass from position through origin.
    EXPECT_VECS_CLOSE(Normalized(ray.origin - f.position), ray.direction);

    // Ray in the lower-right corner
    ray = f.BuildRay(Point2f(1, 0));
    EXPECT_PTS_CLOSE(Point3f(2, -2, 8), ray.origin);
    EXPECT_VECS_CLOSE(Normalized(ray.origin - f.position), ray.direction);

    // Ray in the upper-left corner
    ray = f.BuildRay(Point2f(0, 1));
    EXPECT_PTS_CLOSE(Point3f(-2, 2, 8), ray.origin);
    EXPECT_VECS_CLOSE(Normalized(ray.origin - f.position), ray.direction);

    // Ray in the upper-right corner
    ray = f.BuildRay(Point2f(1, 1));
    EXPECT_PTS_CLOSE(Point3f(2, 2, 8), ray.origin);
    EXPECT_VECS_CLOSE(Normalized(ray.origin - f.position), ray.direction);
}

TEST_F(FrustumTest, BuildRayTransformed) {
    Frustum f;
    f.pnear = 1.f;
    f.position.Set(0, 10, 40);
    Ray ray = f.BuildRay(Point2f(.5f, .5f));
    EXPECT_PTS_CLOSE(Point3f(0, 10, 39),   ray.origin);
    EXPECT_VECS_CLOSE(Vector3f(0, 0, -1), ray.direction);

    // Rotate 90 degrees around the Y axis.
    f.orientation = BuildRotation(Vector3f::AxisY(), 90);
    EXPECT_VECS_CLOSE(Vector3f(-1, 0, 0), f.GetViewDirection());
    ray = f.BuildRay(Point2f(.5f, .5f));
    EXPECT_PTS_CLOSE(Point3f(-1, 10, 40), ray.origin);
    EXPECT_VECS_CLOSE(Vector3f(-1, 0, 0),  ray.direction);
}

TEST_F(FrustumTest, ProjectToNearPlane) {
    // Opposite of BuildRayTransformed.
    Frustum f;
    f.pnear = 1.f;
    f.position.Set(0, 10, 40);
    EXPECT_PTS_CLOSE2(Point2f(.5f, .5f),
                      f.ProjectToImageRect(Point3f(0, 10, 0)));

    f.orientation = BuildRotation(Vector3f::AxisY(), 90);
    EXPECT_PTS_CLOSE2(Point2f(.5f, .5f),
                      f.ProjectToImageRect(Point3f(-30, 10, 40)));
}
