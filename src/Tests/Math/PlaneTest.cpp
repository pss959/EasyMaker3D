//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Plane.h"
#include "Math/Types.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class PlaneTest : public TestBase {};

TEST_F(PlaneTest, GetDistanceToPoint) {
    // Default Plane.
    Plane pl;
    EXPECT_EQ(0,  pl.GetDistanceToPoint(Point3f(0, 0,  0)));
    EXPECT_EQ(5,  pl.GetDistanceToPoint(Point3f(0, 0,  5)));
    EXPECT_EQ(-5, pl.GetDistanceToPoint(Point3f(0, 0, -5)));

    // Plane with distance specified.
    pl = Plane(5, Vector3f::AxisX());
    EXPECT_EQ(-5, pl.GetDistanceToPoint(Point3f( 0, 0, 0)));
    EXPECT_EQ( 0, pl.GetDistanceToPoint(Point3f( 5, 0, 0)));
    EXPECT_EQ( 5, pl.GetDistanceToPoint(Point3f(10, 0, 0)));

    // Plane with point specified.
    pl = Plane(Point3f(5, 0, 0), Vector3f::AxisX());
    EXPECT_EQ(-5, pl.GetDistanceToPoint(Point3f( 0, 0, 0)));
    EXPECT_EQ( 0, pl.GetDistanceToPoint(Point3f( 5, 0, 0)));
    EXPECT_EQ( 5, pl.GetDistanceToPoint(Point3f(10, 0, 0)));
}

TEST_F(PlaneTest, ProjectPoint) {
    Plane pl(0, Vector3f::AxisY());
    EXPECT_EQ(Point3f(2, 0, 3), pl.ProjectPoint(Point3f(2,    0, 3)));
    EXPECT_EQ(Point3f(2, 0, 3), pl.ProjectPoint(Point3f(2,   10, 3)));
    EXPECT_EQ(Point3f(2, 0, 3), pl.ProjectPoint(Point3f(2, -100, 3)));

    pl = Plane(5, Vector3f::AxisX());
    EXPECT_EQ(Point3f(5, 12, 3), pl.ProjectPoint(Point3f(5,   12, 3)));
    EXPECT_EQ(Point3f(5, 12, 3), pl.ProjectPoint(Point3f(2,   12, 3)));
    EXPECT_EQ(Point3f(5, 12, 3), pl.ProjectPoint(Point3f(-13, 12, 3)));
}

TEST_F(PlaneTest, ProjectVector) {
    Plane pl(10, Vector3f::AxisY());
    EXPECT_EQ(Vector3f(2, 0, 3), pl.ProjectVector(Vector3f(2, 11, 3)));
}

TEST_F(PlaneTest, ProjectRotation) {
    const Rotationf rx = BuildRotation(Vector3f::AxisX(),  45);
    const Rotationf ry = BuildRotation(Vector3f::AxisY(), -60);
    const Plane pl(10, Vector3f::AxisY());
    EXPECT_ROTS_CLOSE(BuildRotation(Vector3f::AxisX(), 0),
                      pl.ProjectRotation(rx));
    EXPECT_ROTS_CLOSE(ry, pl.ProjectRotation(ry));
}

TEST_F(PlaneTest, MirrorPoint) {
    Plane pl(5, Vector3f::AxisY());
    EXPECT_EQ(Point3f(2, 10, 3), pl.MirrorPoint(Point3f(2,  0, 3)));
    EXPECT_EQ(Point3f(2,  5, 3), pl.MirrorPoint(Point3f(2,  5, 3)));
    EXPECT_EQ(Point3f(2,  0, 3), pl.MirrorPoint(Point3f(2, 10, 3)));
}

TEST_F(PlaneTest, GetCoefficients) {
    Plane pl(10, Vector3f::AxisZ());
    const Vector4f c = pl.GetCoefficients();

    // Coefficients are a,b,c,d, where ax+by+cz+d=0.
    auto mult = [&](float x, float y, float z){
        return c[0] * x + c[1] * y + c[2] * z + c[3];
    };

    EXPECT_NEAR(0,  mult(0, 0, 10), .00001f);
    EXPECT_NEAR(4,  mult(0, 0, 14), .00001f);
    EXPECT_NEAR(-4, mult(0, 0,  6), .00001f);
}
