#include "Math/Linear.h"
#include "Testing.h"

TEST(LinearTest, RotationDifference) {
    const Rotationf r0 = Rotationf::FromAxisAndAngle(Vector3f(1, 2, -3),
                                                     Anglef::FromDegrees(18));
    const Rotationf r1 = Rotationf::FromAxisAndAngle(Vector3f(-4, 3, 3),
                                                     Anglef::FromDegrees(-22));
    const Rotationf diff = RotationDifference(r0, r1);
    EXPECT_EQ(r1, r0 * diff);
}

TEST(LinearTest, ComputeNormal) {
    EXPECT_EQ(Vector3f(0, 0, 1), ComputeNormal(Point3f(-10, -10, 0),
                                               Point3f( 10, -10, 0),
                                               Point3f(-10,  10, 0)));

    EXPECT_EQ(Vector3f(-1, 0, 0), ComputeNormal(Point3f(-10, -10, -10),
                                                Point3f(-10, -10,  10),
                                                Point3f(-10,  10, -10)));
}


// XXXX Add more from MakerVR
