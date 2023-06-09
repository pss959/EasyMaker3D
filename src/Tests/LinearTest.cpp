#include "Math/Linear.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

class LinearTest : public TestBase {
};

TEST_F(LinearTest, AreClose) {
    const auto nvec = [](float x, float y, float z){
        return ion::math::Normalized(Vector3f(x, y, z)); };

    EXPECT_TRUE(AreClose(12.f,  12.01f, .02f));
    EXPECT_FALSE(AreClose(12.f, 12.02f, .01f));

    // This uses the square of the distance.
    EXPECT_TRUE(AreClose(Vector3f(0, 1, 2),  Vector3f(0, 1, 2.005f), .01f));
    EXPECT_FALSE(AreClose(Vector3f(0, 1, 2), Vector3f(0, 1, 2.2f),   .01f));

    // This uses angles.
    EXPECT_TRUE(AreDirectionsClose(nvec(1, 0, 0), nvec(1, .1f, 0),
                                   Anglef::FromDegrees(10)));
    EXPECT_FALSE(AreDirectionsClose(nvec(1, 0, 0), nvec(1, .5f, 0),
                                    Anglef::FromDegrees(10)));

    EXPECT_TRUE(AreAlmostPerpendicular(nvec(1, 0, 0), nvec(0, 1, 0),
                                       Anglef::FromDegrees(1)));
    EXPECT_TRUE(AreAlmostPerpendicular(nvec(1, 0, 0), nvec(0, -1, 0),
                                       Anglef::FromDegrees(1)));
    EXPECT_FALSE(AreAlmostPerpendicular(nvec(1, 0, 0), nvec(.2f, 1, 0),
                                        Anglef::FromDegrees(1)));
}

TEST_F(LinearTest, MinMaxElement) {
    const Vector2f v2a(-.2f, 1.f);
    const Vector2f v2b( .2f, 1.f);
    const Vector3f v3a(-.2f, 1.f, -1.4f);
    const Vector3f v3b( .2f, 1.f,   .4f);
    const Vector4f v4a(-.2f, 1.f, -.4f, -1.5f);
    const Vector4f v4b( .2f, 1.f,  .1f,   .5f);

    EXPECT_EQ(0, GetMinElementIndex(v2a));
    EXPECT_EQ(0, GetMinElementIndex(v2b));
    EXPECT_EQ(2, GetMinElementIndex(v3a));
    EXPECT_EQ(0, GetMinElementIndex(v3b));
    EXPECT_EQ(3, GetMinElementIndex(v4a));
    EXPECT_EQ(2, GetMinElementIndex(v4b));

    EXPECT_EQ(1, GetMaxElementIndex(v2a));
    EXPECT_EQ(1, GetMaxElementIndex(v2b));
    EXPECT_EQ(1, GetMaxElementIndex(v3a));
    EXPECT_EQ(1, GetMaxElementIndex(v3b));
    EXPECT_EQ(1, GetMaxElementIndex(v4a));
    EXPECT_EQ(1, GetMaxElementIndex(v4b));

    EXPECT_EQ(0, GetMinAbsElementIndex(v2a));
    EXPECT_EQ(0, GetMinAbsElementIndex(v2b));
    EXPECT_EQ(0, GetMinAbsElementIndex(v3a));
    EXPECT_EQ(0, GetMinAbsElementIndex(v3b));
    EXPECT_EQ(0, GetMinAbsElementIndex(v4a));
    EXPECT_EQ(2, GetMinAbsElementIndex(v4b));

    EXPECT_EQ(1, GetMaxAbsElementIndex(v2a));
    EXPECT_EQ(1, GetMaxAbsElementIndex(v2b));
    EXPECT_EQ(2, GetMaxAbsElementIndex(v3a));
    EXPECT_EQ(1, GetMaxAbsElementIndex(v3b));
    EXPECT_EQ(3, GetMaxAbsElementIndex(v4a));
    EXPECT_EQ(1, GetMaxAbsElementIndex(v4b));
}

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

TEST_F(LinearTest, TransformNormal) {
    using ion::math::Cross;
    using ion::math::Dot;
    using ion::math::Length;
    using ion::math::Normalized;

    // Shorthand.
    const Vector3f x = Vector3f::AxisX();
    const Vector3f y = Vector3f::AxisY();
    const Vector3f z = Vector3f::AxisZ();
    const Vector3f v = Normalized(Vector3f(1, 2, 3));

    // Identity.
    EXPECT_VECS_CLOSE(x, TransformNormal(x, Matrix4f::Identity()));
    EXPECT_VECS_CLOSE(y, TransformNormal(y, Matrix4f::Identity()));
    EXPECT_VECS_CLOSE(z, TransformNormal(z, Matrix4f::Identity()));
    EXPECT_VECS_CLOSE(v, TransformNormal(v, Matrix4f::Identity()));

    // Pure rotation.
    const Rotationf rot = Rotationf::RotateInto(x, y);
    const Matrix4f rm(ion::math::RotationMatrixH(rot));
    EXPECT_VECS_CLOSE( y, TransformNormal(x, rm));
    EXPECT_VECS_CLOSE(-x, TransformNormal(y, rm));
    EXPECT_VECS_CLOSE( z, TransformNormal(z, rm));
    EXPECT_VECS_CLOSE(Vector3f(-v[1], v[0], v[2]), TransformNormal(v, rm));
    EXPECT_CLOSE(1, Length(TransformNormal(v, rm)));

    // Pure scale..
    const Matrix4f sm(ion::math::ScaleMatrixH(Vector3f(10, 40, 80)));
    EXPECT_VECS_CLOSE(x, TransformNormal(x, sm));
    EXPECT_VECS_CLOSE(y, TransformNormal(y, sm));
    EXPECT_VECS_CLOSE(z, TransformNormal(z, sm));
    EXPECT_VECS_CLOSE(Vector3f(.847998f, .423999f, .317999f),
                      TransformNormal(v, sm));
    EXPECT_CLOSE(1, Length(TransformNormal(v, sm)));

    // General matrix. Apply to a vector and its perpendicular normal and make
    // sure the results are perpendicular.
    const Vector3f n = Normalized(Cross(v, Vector3f(-2, 1, 5)));
    EXPECT_CLOSE(0, Dot(n, v));
    const Matrix4f gm =
        ion::math::TranslationMatrix(Vector3f(-3, 10, 4)) * rm * sm;
    const Vector3f tv = gm * v;
    const Vector3f tn = TransformNormal(n, gm);
    EXPECT_CLOSE(0, Dot(tn, tv));

    // Invert back to the original.
    EXPECT_VECS_CLOSE(n, TransformNormal(tn, ion::math::Inverse(gm)));
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
    EXPECT_CLOSE(0, pl.GetDistanceToPoint(p0));
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
    EXPECT_CLOSE(30.f, tpl.distance);

    // Do the transformed point test.
    const Point3f p0(10, 0, 0);
    EXPECT_CLOSE(0, pl.GetDistanceToPoint(p0));
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
    EXPECT_CLOSE(0, pl.GetDistanceToPoint(p0));
    const Point3f p1 = p0 + 10 * pl.normal;
    const Point3f tp0 = tm * p0;
    const Point3f tp1 = tm * p1;
    EXPECT_VECS_CLOSE(tpl.normal, ion::math::Normalized(tp1 - tp0));
}

/// \todo Add more from Unity-based version.
