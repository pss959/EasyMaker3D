#include "Math/Bounds.h"
#include "Math/Frustum.h"
#include "Math/Linear.h"
#include "Math/Plane.h"
#include "Math/Ray.h"
#include "Math/Spin.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

#include <ion/math/matrixutils.h>
#include <ion/math/transformutils.h>
#include <ion/math/vectorutils.h>

/// \ingroup Tests
class LinearTest : public TestBase {};

// ----------------------------------------------------------------------------
// Dimension conversion functions.
// ----------------------------------------------------------------------------

TEST_F(LinearTest, TypeConversion) {
    const Vector3f v(13, -4, 10);
    EXPECT_EQ(Vector2f(13, -4), ToVector2f(v));
    EXPECT_EQ(Vector2f(13, 10), ToVector2f(v, 1));
    EXPECT_EQ(Vector2f(-4, 10), ToVector2f(v, 0));

    const Point3f p(13, -4, 10);
    EXPECT_EQ(Point2f(13, -4), ToPoint2f(p));
    EXPECT_EQ(Point2f(13, 10), ToPoint2f(p, 1));
    EXPECT_EQ(Point2f(-4, 10), ToPoint2f(p, 0));

    const Range3f r(Point3f(-2, 5, -7), Point3f(4, 6, 5));
    EXPECT_EQ(Range2f(Point2f(-2,  5), Point2f(4, 6)), ToRange2f(r));
    EXPECT_EQ(Range2f(Point2f(-2, -7), Point2f(4, 5)), ToRange2f(r, 1));
    EXPECT_EQ(Range2f(Point2f(5,  -7), Point2f(6, 5)), ToRange2f(r, 0));
}

TEST_F(LinearTest, DimConversion) {
    EXPECT_EQ(Dim::kX, ToUserDim(Dim::kX));
    EXPECT_EQ(Dim::kZ, ToUserDim(Dim::kY));
    EXPECT_EQ(Dim::kY, ToUserDim(Dim::kZ));

    EXPECT_EQ(Dim::kX, FromUserDim(Dim::kX));
    EXPECT_EQ(Dim::kZ, FromUserDim(Dim::kY));
    EXPECT_EQ(Dim::kY, FromUserDim(Dim::kZ));
}

// ----------------------------------------------------------------------------
// Transformation functions.
// ----------------------------------------------------------------------------

TEST_F(LinearTest, GetTransformMatrix) {
    const Matrix4f tm =
        GetTransformMatrix(Vector3f(2, 3, 4),
                           BuildRotation(Vector3f::AxisY(), 90),
                           Vector3f(10, 20, 30));
    const Matrix4f expected( 0, 0, 4, 10,
                             0, 3, 0, 20,
                            -2, 0, 0, 30,
                             0, 0, 0,  1);
    EXPECT_MATS_CLOSE(expected, tm);
}

TEST_F(LinearTest, TransformNormal) {
    using ion::math::Cross;
    using ion::math::Dot;
    using ion::math::Length;

    // Shorthand.
    const Vector3f x = Vector3f::AxisX();
    const Vector3f y = Vector3f::AxisY();
    const Vector3f z = Vector3f::AxisZ();
    const Vector3f v = Normalized(1, 2, 3);

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

TEST_F(LinearTest, TransformRay) {
    const Matrix4f m =
        GetTransformMatrix(Vector3f(2, 3, 4),
                           BuildRotation(Vector3f::AxisY(), 90),
                           Vector3f(10, 20, 30));

    const Ray r = TransformRay(Ray(Point3f(1, 1, 1), -Vector3f::AxisZ()), m);
    EXPECT_PTS_CLOSE(Point3f(14, 23, 28), r.origin);
    EXPECT_VECS_CLOSE(Vector3f(-4, 0, 0), r.direction);
}

TEST_F(LinearTest, TranslatePlane) {
    const Plane pl(10, Vector3f::AxisZ());

    // Translating parallel to plane has no effect.
    EXPECT_EQ(pl, TranslatePlane(pl, Vector3f(3,  0, 0)));
    EXPECT_EQ(pl, TranslatePlane(pl, Vector3f(0, -5, 0)));

    EXPECT_EQ(Plane(13, Vector3f::AxisZ()),
              TranslatePlane(pl, Vector3f(0, 0, 3)));
    EXPECT_EQ(Plane(7,  Vector3f::AxisZ()),
              TranslatePlane(pl, Vector3f(0, 0, -3)));
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
    EXPECT_VECS_CLOSE(tpl.normal, Normalized(tp1 - tp0));
}
TEST_F(LinearTest, TransformPlane2) {
    // Rotate and translate a plane not at the origin.
    const Plane pl(10, Vector3f::AxisX());

    const Rotationf rot = BuildRotation(Vector3f::AxisZ(), 90);
    const Matrix4f  tm  = GetTransformMatrix(Vector3f(1, 1, 1), rot,
                                             Vector3f(10, 20, 30));
    const Plane tpl = TransformPlane(pl, tm);
    EXPECT_VECS_CLOSE(Vector3f::AxisY(), tpl.normal);
    EXPECT_CLOSE(30.f, tpl.distance);

    // Do the transformed point test.
    const Point3f p0(10, 0, 0);
    EXPECT_CLOSE(0, pl.GetDistanceToPoint(p0));
    const Point3f p1 = p0 + 10 * pl.normal;
    const Point3f tp0 = tm * p0;
    const Point3f tp1 = tm * p1;
    EXPECT_VECS_CLOSE(tpl.normal, Normalized(tp1 - tp0));
}

TEST_F(LinearTest, TransformPlane3) {
    // Scale, rotate, and translate a plane not at the origin.
    const Plane pl(10, Vector3f::AxisX());

    const Rotationf rot = BuildRotation(1, 2, 3, 40);
    const Matrix4f  tm  = GetTransformMatrix(Vector3f(2, 5, 10),
                                             rot, Vector3f(10, 20, 30));
    const Plane     tpl = TransformPlane(pl, tm);

    // Do the transformed point test.
    const Point3f p0(10, 0, 0);
    EXPECT_CLOSE(0, pl.GetDistanceToPoint(p0));
    const Point3f p1 = p0 + 10 * pl.normal;
    const Point3f tp0 = tm * p0;
    const Point3f tp1 = tm * p1;
    EXPECT_VECS_CLOSE(tpl.normal, Normalized(tp1 - tp0));
}

TEST_F(LinearTest, TransformPlane4) {
    // Testing adjusting normals near axis directions.
    const Matrix4f tiny_rot =
        ion::math::RotationMatrixH(BuildRotation(1, 1, 1, .001f));
    const auto test_axis = [&](const Vector3f &axis){
        const Plane p(0, axis);
        EXPECT_EQ(p, TransformPlane(p, tiny_rot));
    };
    test_axis( Vector3f::AxisX());
    test_axis(-Vector3f::AxisX());
    test_axis( Vector3f::AxisY());
    test_axis(-Vector3f::AxisY());
    test_axis( Vector3f::AxisZ());
    test_axis(-Vector3f::AxisZ());
}

TEST_F(LinearTest, TransformBounds) {
    const Bounds b(Point3f(-4, -3, 6), Point3f(-2, 9, 12));

    EXPECT_EQ(Bounds(Point3f(-12, -12, 30), Point3f(-6, 36, 60)),
              ScaleBounds(b, Vector3f(3, 4, 5)));

    EXPECT_EQ(Bounds(Point3f(-104, 197, 406), Point3f(-102, 209, 412)),
              TranslateBounds(b, Vector3f(-100, 200, 400)));

    const Matrix4f tm =
        GetTransformMatrix(Vector3f(2, 3, 4),
                           BuildRotation(Vector3f::AxisY(), 90),
                           Vector3f(10, 20, 30));
    const Bounds tb = TransformBounds(b, tm);
    EXPECT_PTS_CLOSE(Point3f(34, 11, 34), tb.GetMinPoint());
    EXPECT_PTS_CLOSE(Point3f(58, 47, 38), tb.GetMaxPoint());
}

TEST_F(LinearTest, TransformSpin) {
    Spin s;
    s.center.Set(1, 2, 3);
    s.axis = Vector3f::AxisX();
    s.angle = Anglef::FromDegrees(30);
    s.offset = 12;
    const Matrix4f tm =
        GetTransformMatrix(Vector3f(2, 3, 4),
                           BuildRotation(Vector3f::AxisY(), 90),
                           Vector3f(10, 20, 30));
    const Spin ts = TransformSpin(s, tm);
    EXPECT_PTS_CLOSE(Point3f(22, 26, 28), ts.center);
    EXPECT_VECS_CLOSE(-Vector3f::AxisZ(), ts.axis);
    EXPECT_EQ(s.angle,                    ts.angle);
    EXPECT_EQ(s.offset,                   ts.offset);
}

TEST_F(LinearTest, TransformRotation) {
    const Rotationf r0 = Rotationf::RotateInto(-Vector3f::AxisZ(),
                                               Vector3f::AxisX());
    const Rotationf r1 = Rotationf::RotateInto(Vector3f::AxisX(),
                                               Vector3f::AxisY());
    const Rotationf r = TransformRotation(r0, ion::math::RotationMatrixH(r1));
    EXPECT_ROTS_CLOSE(Rotationf::RotateInto(-Vector3f::AxisZ(),
                                            Vector3f::AxisY()), r);
}

TEST_F(LinearTest, ComposeRotations) {
    const Rotationf r0 = Rotationf::RotateInto(Vector3f::AxisX(),
                                               Vector3f::AxisY());
    const Rotationf r1 = Rotationf::RotateInto(Vector3f::AxisY(),
                                               Vector3f::AxisZ());

    // r0 should be applied first. Should bring +X to +Z and +Y to -X.
    const Rotationf cr0 = ComposeRotations(r0, r1);
    EXPECT_VECS_CLOSE( Vector3f::AxisZ(), cr0 * Vector3f::AxisX());
    EXPECT_VECS_CLOSE(-Vector3f::AxisX(), cr0 * Vector3f::AxisY());

    // r1 should be applied first. Should bring +X to +Y and +Y to +Z.
    const Rotationf cr1 = ComposeRotations(r1, r0);
    EXPECT_VECS_CLOSE(Vector3f::AxisY(), cr1 * Vector3f::AxisX());
    EXPECT_VECS_CLOSE(Vector3f::AxisZ(), cr1 * Vector3f::AxisY());
}

TEST_F(LinearTest, ScalePointsToSize) {
    // This also tests ScalePoint().

    // 2D version.
    std::vector<Point2f> p2{ Point2f(0, 0), Point2f(-1, 1), Point2f(4, -4) };
    ScalePointsToSize(Vector2f(20, 30), p2);
    EXPECT_PTS_CLOSE2(Point2f(0,    0), p2[0]);
    EXPECT_PTS_CLOSE2(Point2f(-4,   6), p2[1]);
    EXPECT_PTS_CLOSE2(Point2f(16, -24), p2[2]);

    // 3D version.
    const std::vector<Point3f> p3{
        Point3f(0, 0, 0), Point3f(-1, 1, 1), Point3f(4, -4, 4)
    };
    auto p3a = p3;
    ScalePointsToSize(Vector3f(20, 30, 20), p3a);
    EXPECT_PTS_CLOSE(Point3f(0,    0,  0),  p3a[0]);
    EXPECT_PTS_CLOSE(Point3f(-4,   6,  5),  p3a[1]);
    EXPECT_PTS_CLOSE(Point3f(16, -24, 20),  p3a[2]);
    auto p3b = p3;
    ScalePointsToSize(Vector3f(5, 5, 20), p3b);
    EXPECT_PTS_CLOSE(Point3f(0,   0,  0), p3b[0]);
    EXPECT_PTS_CLOSE(Point3f(-1,  1,  5), p3b[1]);
    EXPECT_PTS_CLOSE(Point3f(4,  -4, 20), p3b[2]);
}

TEST_F(LinearTest, ToPrintCoords) {
    EXPECT_EQ(Point3f(4, -5, 6),  ToPrintCoords(Point3f(4, 6, 5)));
    EXPECT_EQ(Vector3f(4, -5, 6), ToPrintCoords(Vector3f(4, 6, 5)));
}

// ----------------------------------------------------------------------------
// Viewing functions.
// ----------------------------------------------------------------------------

TEST_F(LinearTest, GetAspectRatio) {
    EXPECT_EQ(4.5f, GetAspectRatio(Viewport(Point2ui(0, 0),
                                            Point2ui(450, 100))));
    EXPECT_EQ(.5f,  GetAspectRatio(Viewport(Point2ui(100, 200),
                                            Point2ui(400, 800))));
}

TEST_F(LinearTest, GetProjectionAndViewMatrix) {
    Frustum f;
    f.SetSymmetricFOV(Anglef::FromDegrees(90.f), 2.f);
    f.pnear = 1;
    f.pfar  = 100;
    f.position.Set(0, 10, 40);
    f.orientation = Rotationf::RotateInto(Vector3f::AxisX(), Vector3f::AxisY());

    EXPECT_MATS_CLOSE(Matrix4f(1, 0, 0, 0,
                               0, 2, 0, 0,
                               0, 0, -1.0101f, -1.0101f,
                               0, 0, -1, 0), GetProjectionMatrix(f));

    EXPECT_MATS_CLOSE(Matrix4f(0, 1, 0, -10,
                               -1, 0, 0, 0,
                               0, 0, 1, -40,
                               0, 0, 0, 1), GetViewMatrix(f));
}

// ----------------------------------------------------------------------------
// General linear algebra functions.
// ----------------------------------------------------------------------------

TEST_F(LinearTest, ZeroAndOneInit) {
    EXPECT_EQ(0.f, ZeroInit<float>());
    EXPECT_EQ(1.f,  OneInit<float>());

    EXPECT_EQ(Vector2f(0, 0), ZeroInit<Vector2f>());
    EXPECT_EQ(Vector2f(1, 1),  OneInit<Vector2f>());
}

TEST_F(LinearTest, RoundToPrecision) {
    EXPECT_CLOSE(.05f, RoundToPrecision(.05493f, .01f));
    EXPECT_CLOSE(.05f, RoundToPrecision(.04792f, .01f));
}

TEST_F(LinearTest, AreClose) {
    EXPECT_TRUE(AreClose(12.f,  12.01f, .02f));
    EXPECT_FALSE(AreClose(12.f, 12.02f, .01f));

    // This uses the square of the distance.
    EXPECT_TRUE(AreClose(Vector3f(0, 1, 2),  Vector3f(0, 1, 2.005f), .01f));
    EXPECT_FALSE(AreClose(Vector3f(0, 1, 2), Vector3f(0, 1, 2.2f),   .01f));

    EXPECT_TRUE(AreClose(Anglef::FromDegrees(12), Anglef::FromDegrees(12.9f),
                         Anglef::FromDegrees(1)));
    EXPECT_FALSE(AreClose(Anglef::FromDegrees(12), Anglef::FromDegrees(13.1f),
                          Anglef::FromDegrees(1)));

    // This uses angles.
    EXPECT_TRUE(AreDirectionsClose(Normalized(1, 0, 0), Normalized(1, .1f, 0),
                                   Anglef::FromDegrees(10)));
    EXPECT_FALSE(AreDirectionsClose(Normalized(1, 0, 0), Normalized(1, .5f, 0),
                                    Anglef::FromDegrees(10)));

    EXPECT_TRUE(AreAlmostPerpendicular(Normalized(1, 0, 0),
                                       Normalized(0, 1, 0),
                                       Anglef::FromDegrees(1)));
    EXPECT_TRUE(AreAlmostPerpendicular(Normalized(1, 0, 0),
                                       Normalized(0, -1, 0),
                                       Anglef::FromDegrees(1)));
    EXPECT_FALSE(AreAlmostPerpendicular(Normalized(1, 0, 0),
                                        Normalized(.2f, 1, 0),
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

TEST_F(LinearTest, GetAxis) {
    EXPECT_EQ(Vector3f(1,   0, 0), GetAxis(Dim::kX));
    EXPECT_EQ(Vector3f(2.5, 0, 0), GetAxis(Dim::kX, 2.5));
    EXPECT_EQ(Vector3f(0, 1,   0), GetAxis(Dim::kY));
    EXPECT_EQ(Vector3f(0, 2.5, 0), GetAxis(Dim::kY, 2.5));
    EXPECT_EQ(Vector3f(0, 0, 1),   GetAxis(Dim::kZ));
    EXPECT_EQ(Vector3f(0, 0, 2.5), GetAxis(Dim::kZ, 2.5));

    EXPECT_EQ(Vector3f(1,   0, 0), GetAxis(0));
    EXPECT_EQ(Vector3f(2.5, 0, 0), GetAxis(0, 2.5));
    EXPECT_EQ(Vector3f(0, 1,   0), GetAxis(1));
    EXPECT_EQ(Vector3f(0, 2.5, 0), GetAxis(1, 2.5));
    EXPECT_EQ(Vector3f(0, 0, 1),   GetAxis(2));
    EXPECT_EQ(Vector3f(0, 0, 2.5), GetAxis(2, 2.5));
}

TEST_F(LinearTest, AbsAngle) {
    EXPECT_EQ(Anglef::FromDegrees(12), AbsAngle(Anglef::FromDegrees(12)));
    EXPECT_EQ(Anglef::FromDegrees(12), AbsAngle(Anglef::FromDegrees(-12)));
}

TEST_F(LinearTest, NormalizedAngle) {
    const auto ndeg = [](float deg){
        return NormalizedAngle(Anglef::FromDegrees(deg)).Degrees();
    };
    EXPECT_CLOSE(0.f,   ndeg(0));
    EXPECT_CLOSE(359.f, ndeg(359));
    EXPECT_CLOSE(0.f,   ndeg(360));
    EXPECT_CLOSE(2.f,   ndeg(722));
    EXPECT_CLOSE(0.f,   ndeg(-360));
    EXPECT_CLOSE(10.f,  ndeg(-350));
}

TEST_F(LinearTest, RotationAngleAndAxis) {
    const Rotationf rot = BuildRotation(1, 2, 3, 12);
    EXPECT_CLOSE(12.f, RotationAngle(rot).Degrees());
    EXPECT_VECS_CLOSE(Normalized(1, 2, 3), RotationAxis(rot));
}

TEST_F(LinearTest, RotationFromMatrix) {
    const Rotationf rot = BuildRotation(1, 2, 3, 12);
    const Matrix4f  tm  = GetTransformMatrix(Vector3f(2, 3, 4), rot,
                                             Vector3f(10, 20, 30));
    EXPECT_ROTS_CLOSE(rot, RotationFromMatrix(tm));
}

TEST_F(LinearTest, RotationDifference) {
    const Rotationf r0 = BuildRotation(1, 2, -3,  18);
    const Rotationf r1 = BuildRotation(-4, 3, 3, -22);
    const Rotationf diff = RotationDifference(r0, r1);
    EXPECT_EQ(r1, r0 * diff);
}

TEST_F(LinearTest, SignedDistance) {
    EXPECT_EQ(0.f,  SignedDistance(Point3f(0, 0, 0),   Vector3f(0, 0, 0)));
    EXPECT_EQ(0.f,  SignedDistance(Point3f(0, 0, 0),   Vector3f(1, 0, 0)));
    EXPECT_EQ(1.f,  SignedDistance(Point3f(1, 0, 0),   Vector3f(1, 0, 0)));
    EXPECT_EQ(-1.f, SignedDistance(Point3f(-1, 0, 0),  Vector3f(1, 0, 0)));
    EXPECT_EQ(-1.f, SignedDistance(Point3f(1, 0, 0),   Vector3f(-1, 0, 0)));
    EXPECT_EQ(0.f,  SignedDistance(Point3f(1, 0, 0),   Vector3f(0, 1, 0)));
    EXPECT_EQ(10.f, SignedDistance(Point3f(10, 0, 0),  Vector3f(1, 2, 0)));
    EXPECT_EQ(-8.f, SignedDistance(Point3f(10, -4, 0), Vector3f(0, 2, 0)));
}

TEST_F(LinearTest, ComputeNormal) {
    EXPECT_EQ(Vector3f(0, 0, 1), ComputeNormal(Point3f(-10, -10, 0),
                                               Point3f( 10, -10, 0),
                                               Point3f(-10,  10, 0)));

    const std::vector<Point3f> pts{
        Point3f(-10, -10, -10),
        Point3f(-10, -10,  10),
        Point3f(-10,  10, -10)
    };
    EXPECT_EQ(Vector3f(-1, 0, 0), ComputeNormal(pts));
}

TEST_F(LinearTest, ComputeArea) {
    EXPECT_EQ(27.f, ComputeArea(Point3f(-3, -1, 0),
                                Point3f(6, -1, 0),
                                Point3f(6, 5, 0)));
    const std::vector<Point3f> pts{
        Point3f(-10, 2, 1),
        Point3f( 10, 2, 1),
        Point3f( 10, 8, 1)
    };
    EXPECT_EQ(60.f, ComputeArea(pts));
}

TEST_F(LinearTest, ComputeBarycentric) {
    const Point2f a(2, 6), b(10, 6), c(10, 10), z(0, 0);
    Vector3f bary;
    EXPECT_TRUE(ComputeBarycentric(Point2f(8, 8), a, b, c, bary));
    EXPECT_VECS_CLOSE(Vector3f(.25f, .25f, .5f), bary);
    EXPECT_FALSE(ComputeBarycentric(Point2f(2, 0), a, b, c, bary));

    EXPECT_FALSE(ComputeBarycentric(Point2f(2, 2), z, z, z, bary));
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

TEST_F(LinearTest, GetClosestLinePoints) {
    Point3f cp0;
    Point3f cp1;

    // Parallel lines.
    EXPECT_FALSE(GetClosestLinePoints(Point3f::Zero(),  Vector3f::AxisZ(),
                                      Point3f(1, 1, 1), Vector3f::AxisZ(),
                                      cp0, cp1));

    EXPECT_TRUE(GetClosestLinePoints(Point3f(0, 4, 0),  Vector3f::AxisX(),
                                     Point3f(3, 0, 0), -Vector3f::AxisZ(),
                                     cp0, cp1));
    EXPECT_PTS_CLOSE(Point3f(3, 4, 0), cp0);
    EXPECT_PTS_CLOSE(Point3f(3, 0, 0), cp1);
}

TEST_F(LinearTest, IsNearLineSegment) {
    EXPECT_TRUE(IsNearLineSegment(Point2f(0, .09f),
                                  Point2f(-10, 0), Point2f(20, 0), .1f));
    EXPECT_TRUE(IsNearLineSegment(Point2f(5.01f, 4.98f),
                                  Point2f(-10, -10), Point2f(20, 20), .1f));
    EXPECT_FALSE(IsNearLineSegment(Point2f(5.01f, 4.6f),
                                   Point2f(-10, -10), Point2f(20, 20), .1f));
    EXPECT_FALSE(IsNearLineSegment(Point2f(0, 0),
                                   Point2f(0, 0), Point2f(0, 0), .1f));
}

TEST_F(LinearTest, BuildRange) {
    const Range2f r = BuildRange(Point2f(-10, 20), Vector2f(6, 8));
    EXPECT_EQ(Point2f(-13, 16), r.GetMinPoint());
    EXPECT_EQ(Point2f( -7, 24), r.GetMaxPoint());
}

TEST_F(LinearTest, MinMaxComponents) {
    const Vector2f v2a(1, 20),    v2b(4, -2);
    const Vector3f v3a(1, 20, 4), v3b(4, -2, 0);
    EXPECT_EQ(Vector2f(1, -2),    MinComponents(v2a, v2b));
    EXPECT_EQ(Vector2f(4, 20),    MaxComponents(v2a, v2b));
    EXPECT_EQ(Vector3f(1, -2, 0), MinComponents(v3a, v3b));
    EXPECT_EQ(Vector3f(4, 20, 4), MaxComponents(v3a, v3b));
    EXPECT_EQ(Point2f(1, -2),     MinComponents(Point2f(v2a), Point2f(v2b)));
    EXPECT_EQ(Point2f(4, 20),     MaxComponents(Point2f(v2a), Point2f(v2b)));
    EXPECT_EQ(Point3f(1, -2, 0),  MinComponents(Point3f(v3a), Point3f(v3b)));
    EXPECT_EQ(Point3f(4, 20, 4),  MaxComponents(Point3f(v3a), Point3f(v3b)));
}

TEST_F(LinearTest, Clamp) {
    EXPECT_EQ(6,   Clamp(6, 6, 14));
    EXPECT_EQ(14,  Clamp(14, 6, 14));
    EXPECT_EQ(6,   Clamp(-6, 6, 14));
    EXPECT_EQ(14,  Clamp(22, 6, 14));
    EXPECT_EQ(-6,  Clamp(-99, -6, 14));

    EXPECT_EQ(6.f,   Clamp(6.f, 6.f, 14.f));
    EXPECT_EQ(14.f,  Clamp(14.f, 6.f, 14.f));
    EXPECT_EQ(6.f,   Clamp(-6.f, 6.f, 14.f));
    EXPECT_EQ(14.f,  Clamp(22.f, 6.f, 14.f));
    EXPECT_EQ(-6.f,  Clamp(-99.f, -6.f, 14.f));

    EXPECT_EQ(Vector2f(4, 5), Clamp(Vector2f(12, -4),
                                    Vector2f(0, 5), Vector2f(4, 15)));
    EXPECT_EQ(Vector3f(4, 5, 6), Clamp(Vector3f(12, -4, 16),
                                       Vector3f(0, 5, 5), Vector3f(4, 15, 6)));

    EXPECT_EQ(Point2f(4, 5), Clamp(Point2f(12, -4),
                                   Point2f(0, 5), Point2f(4, 15)));
    EXPECT_EQ(Point3f(4, 5, 6), Clamp(Point3f(12, -4, 16),
                                      Point3f(0, 5, 5), Point3f(4, 15, 6)));
}

// ----------------------------------------------------------------------------
// Interpolation.
// ----------------------------------------------------------------------------

TEST_F(LinearTest, LerpInt) {
    EXPECT_EQ(13, LerpInt(0,   13, 22));
    EXPECT_EQ(22, LerpInt(1,   13, 22));
    EXPECT_EQ(18, LerpInt(.5f, 13, 22));
}
