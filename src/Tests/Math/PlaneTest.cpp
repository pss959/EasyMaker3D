#include "Math/Types.h"
#include "Tests/Testing.h"

TEST(PlaneTest, GetDistanceToPoint) {
    // Plane with distance specified.
    Plane pl(5, Vector3f::AxisX());
    EXPECT_EQ(-5, pl.GetDistanceToPoint(Point3f( 0, 0, 0)));
    EXPECT_EQ( 0, pl.GetDistanceToPoint(Point3f( 5, 0, 0)));
    EXPECT_EQ( 5, pl.GetDistanceToPoint(Point3f(10, 0, 0)));

    // Plane with point specified.
    pl = Plane(Point3f(5, 0, 0), Vector3f::AxisX());
    EXPECT_EQ(-5, pl.GetDistanceToPoint(Point3f( 0, 0, 0)));
    EXPECT_EQ( 0, pl.GetDistanceToPoint(Point3f( 5, 0, 0)));
    EXPECT_EQ( 5, pl.GetDistanceToPoint(Point3f(10, 0, 0)));
}

TEST(PlaneTest, ProjectPoint) {
    Plane pl(0, Vector3f::AxisY());
    EXPECT_EQ(Point3f(2, 0, 3), pl.ProjectPoint(Point3f(2,    0, 3)));
    EXPECT_EQ(Point3f(2, 0, 3), pl.ProjectPoint(Point3f(2,   10, 3)));
    EXPECT_EQ(Point3f(2, 0, 3), pl.ProjectPoint(Point3f(2, -100, 3)));

    pl = Plane(5, Vector3f::AxisX());
    EXPECT_EQ(Point3f(5, 12, 3), pl.ProjectPoint(Point3f(5,   12, 3)));
    EXPECT_EQ(Point3f(5, 12, 3), pl.ProjectPoint(Point3f(2,   12, 3)));
    EXPECT_EQ(Point3f(5, 12, 3), pl.ProjectPoint(Point3f(-13, 12, 3)));
}

TEST(PlaneTest, ProjectVector) {
    Plane pl(10, Vector3f::AxisY());
    EXPECT_EQ(Vector3f(2, 0, 3), pl.ProjectVector(Vector3f(2, 11, 3)));
}

TEST(PlaneTest, MirrorPoint) {
    Plane pl(5, Vector3f::AxisY());
    EXPECT_EQ(Point3f(2, 10, 3), pl.MirrorPoint(Point3f(2,  0, 3)));
    EXPECT_EQ(Point3f(2,  5, 3), pl.MirrorPoint(Point3f(2,  5, 3)));
    EXPECT_EQ(Point3f(2,  0, 3), pl.MirrorPoint(Point3f(2, 10, 3)));
}

TEST(PlaneTest, GetCoefficients) {
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
