#include "Math/Types.h"
#include "Testing.h"

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
