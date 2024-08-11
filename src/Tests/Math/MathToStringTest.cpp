//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/ToString.h"
#include "Math/Types.h"
#include "Tests/Testing.h"
#include "Util/String.h"

TEST(MathToStringTest, ToStr) {
    EXPECT_EQ("P[1, 3, -2]", ToStr(Point3f(1, 3, -2)));
    EXPECT_EQ("V[1, 3, -2]", ToStr(Vector3f(1, 3, -2)));
    EXPECT_EQ("20 deg",      ToStr(Anglef::FromDegrees(20)));
    EXPECT_EQ("ROT[V[0, 1, 0]: 30 deg]",
              ToStr(Rotationf::FromAxisAndAngle(Vector3f(0, 1, 0),
                                                Anglef::FromDegrees(30))));
}

TEST(MathToStringTest, ToStringPrecision) {
    const float f = 5.3728f;
    EXPECT_EQ("5",      Math::ToString(f, 1.f));
    EXPECT_EQ("5.4",    Math::ToString(f, .1f));
    EXPECT_EQ("5.37",   Math::ToString(f, .01f));
    EXPECT_EQ("5.373",  Math::ToString(f, .001f));
    EXPECT_EQ("5.3728", Math::ToString(f, .0001f));
    EXPECT_EQ("5.3728", Math::ToString(f, .00001f));

    EXPECT_EQ("[5.4, 5.4]",      Math::ToString(Point2f(f, f),     .1f));
    EXPECT_EQ("[5.4, 5.4, 5.4]", Math::ToString(Point3f(f, f, f),  .1f));
    EXPECT_EQ("[5.4, 5.4]",      Math::ToString(Vector2f(f, f),    .1f));
    EXPECT_EQ("[5.4, 5.4, 5.4]", Math::ToString(Vector3f(f, f, f), .1f));

    // Rotation uses a fixed precision.
    EXPECT_EQ("40.1d / [-0.267, -0.535, -0.802]",
              Math::ToString(Rotationf::FromAxisAndAngle(
                                 Vector3f(1, 2, 3.00001f),
                                 Anglef::FromDegrees(-40.1111f))));

    Matrix4f m;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            m[i][j] = 4 * i + j + .01f;
    EXPECT_EQ("[      0      1      2      3\n"
              "       4      5      6      7\n"
              "       8      9     10     11\n"
              "      12     13     14     15 ]\n", Math::ToString(m, .1f));
}
