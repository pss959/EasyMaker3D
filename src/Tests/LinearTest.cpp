#include "Math/Linear.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

#include <ion/math/transformutils.h>

class LinearTest : public TestBase {
};

TEST_F(LinearTest, RotationDifference) {
    const Rotationf r0 = Rotationf::FromAxisAndAngle(Vector3f(1, 2, -3),
                                                     Anglef::FromDegrees(18));
    const Rotationf r1 = Rotationf::FromAxisAndAngle(Vector3f(-4, 3, 3),
                                                     Anglef::FromDegrees(-22));
    const Rotationf diff = RotationDifference(r0, r1);
    EXPECT_EQ(r1, r0 * diff);
}

TEST_F(LinearTest, ComputeNormal) {
    EXPECT_EQ(Vector3f(0, 0, 1), ComputeNormal(Point3f(-10, -10, 0),
                                               Point3f( 10, -10, 0),
                                               Point3f(-10,  10, 0)));

    EXPECT_EQ(Vector3f(-1, 0, 0), ComputeNormal(Point3f(-10, -10, -10),
                                                Point3f(-10, -10,  10),
                                                Point3f(-10,  10, -10)));
}

TEST_F(LinearTest, GetClosestPointOnLine) {
    EXPECT_EQ(Point3f(3, 10, 2), GetClosestPointOnLine(Point3f(3, 20, 2),
                                                       Point3f(0, 10, 2),
                                                       Vector3f(1, 0, 0)));
    EXPECT_EQ(Point3f(3, 10, 2), GetClosestPointOnLine(Point3f(3, 20, 2),
                                                       Point3f(0, 10, 2),
                                                       Vector3f(-1, 0, 0)));
    // Diagonal
    EXPECT_EQ(Point3f(5, 5, 0), GetClosestPointOnLine(Point3f(-3, -3, 0),
                                                       Point3f(10, 0, 0),
                                                       Vector3f(-10, 10, 0)));
}

TEST_F(LinearTest, TransformPlane) {
    const Plane pl(10, Vector3f::AxisZ());
    EXPECT_EQ(Vector3f::AxisZ(), pl.normal);
    EXPECT_EQ(-10, pl.GetDistanceToPoint(Point3f::Zero()));

    const Matrix4f tm = ion::math::TranslationMatrix(Vector3f(1, 2, 3));
    const Plane tpl = TransformPlane(pl, tm);
    EXPECT_EQ(Vector3f::AxisZ(), tpl.normal);
    EXPECT_EQ(-13, tpl.GetDistanceToPoint(Point3f::Zero()));

    // Transform a point on the untransformed plane and the point 10 units
    // along the normal from it. The normalized difference vector between the
    // resulting points should be the transformed normal.
    const Point3f p0(0, 0, 10);
    EXPECT_NEAR(0, pl.GetDistanceToPoint(p0), kClose);
    const Point3f p1 = p0 + 10 * pl.normal;
    const Point3f tp0 = tm * p0;
    const Point3f tp1 = tm * p1;
    EXPECT_VECS_CLOSE(tpl.normal, ion::math::Normalized(tp1 - tp0));
}

TEST_F(LinearTest, TransformPlane2) {
    // Rotate and translate a plane not at the origin.
    const Plane pl(10, Vector3f::AxisX());

    const Rotationf rot = Rotationf:: FromAxisAndAngle(Vector3f::AxisZ(),
                                                       Anglef::FromDegrees(90));
    const Matrix4f tm =
        ion::math::TranslationMatrix(Vector3f(10, 20, 30)) *
        ion::math::RotationMatrixH(rot);
    const Plane tpl = TransformPlane(pl, tm);
    EXPECT_VECS_CLOSE(Vector3f::AxisY(), tpl.normal);
    EXPECT_NEAR(30.f, tpl.distance, kClose);

    // Do the transformed point test.
    const Point3f p0(10, 0, 0);
    EXPECT_NEAR(0, pl.GetDistanceToPoint(p0), kClose);
    const Point3f p1 = p0 + 10 * pl.normal;
    const Point3f tp0 = tm * p0;
    const Point3f tp1 = tm * p1;
    EXPECT_VECS_CLOSE(tpl.normal, ion::math::Normalized(tp1 - tp0));
}

TEST_F(LinearTest, TransformPlane3) {
    // Scale, rotate, and translate a plane not at the origin.
    const Plane pl(10, Vector3f::AxisX());

    const Rotationf rot = Rotationf:: FromAxisAndAngle(Vector3f(1, 2, 3),
                                                       Anglef::FromDegrees(40));
    const Matrix4f tm =
        ion::math::TranslationMatrix(Vector3f(10, 20, 30)) *
        ion::math::RotationMatrixH(rot) *
        ion::math::ScaleMatrixH(Vector3f(2, 5, 10));
    const Plane tpl = TransformPlane(pl, tm);

    // Do the transformed point test.
    const Point3f p0(10, 0, 0);
    EXPECT_NEAR(0, pl.GetDistanceToPoint(p0), kClose);
    const Point3f p1 = p0 + 10 * pl.normal;
    const Point3f tp0 = tm * p0;
    const Point3f tp1 = tm * p1;
    EXPECT_VECS_CLOSE(tpl.normal, ion::math::Normalized(tp1 - tp0));
}

/// \todo Add more from old MakerVR
