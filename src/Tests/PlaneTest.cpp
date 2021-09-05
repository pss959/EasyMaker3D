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
