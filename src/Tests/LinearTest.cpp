#include "Math/Linear.h"
#include "TestBase.h"
#include "Testing.h"

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

TEST_F(LinearTest, TransformPlane) {
    const Plane pl(10, Vector3f::AxisZ());
    EXPECT_EQ(Vector3f::AxisZ(), pl.normal);
    EXPECT_EQ(-10, pl.GetDistanceToPoint(Point3f::Zero()));

    const Matrix4f tm = ion::math::TranslationMatrix(Vector3f(1, 2, 3));
    const Plane tpl = TransformPlane(pl, tm);
    EXPECT_EQ(Vector3f::AxisZ(), tpl.normal);
    EXPECT_EQ(-13, tpl.GetDistanceToPoint(Point3f::Zero()));
}

TEST_F(LinearTest, TransformPlane2) {
    // Rotate and translate a plane not at the origin.
    const Plane pl(10, Vector3f::AxisX());

    const Rotationf rot = Rotationf:: FromAxisAndAngle(Vector3f::AxisZ(),
                                                       Anglef::FromDegrees(90));
    const Matrix4f m =
        ion::math::TranslationMatrix(Vector3f(10, 20, 30)) *
        ion::math::RotationMatrixH(rot);
    const Plane tpl = TransformPlane(pl, m);
    EXPECT_VECS_CLOSE(Vector3f::AxisY(), tpl.normal);
    EXPECT_NEAR(30.f, tpl.distance, kClose);
}

/// \todo Add more from old MakerVR
