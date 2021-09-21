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

// XXXX Add more from MakerVR
