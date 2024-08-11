//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Spin.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class SpinTest : public TestBase {};

TEST_F(SpinTest, DefaultSpin) {
    const Spin s;
    EXPECT_EQ(Point3f::Zero(),        s.center);
    EXPECT_EQ(Vector3f::AxisY(),      s.axis);
    EXPECT_EQ(Anglef::FromDegrees(0), s.angle);
    EXPECT_EQ(0.f,                    s.offset);
}

TEST_F(SpinTest, Equality) {
    Spin s0;
    Spin s1;
    Spin s2;
    Spin s3;
    Spin s4;

    // Default spins are equal.
    EXPECT_EQ(s0, s1);

    s1.center.Set(.2f, .3f, .4f);
    EXPECT_EQ(s1, s1);
    EXPECT_NE(s0, s1);

    s2.axis.Set(1, 0, 0);
    EXPECT_EQ(s2, s2);
    EXPECT_NE(s0, s2);
    EXPECT_NE(s1, s2);

    s3.angle = Anglef::FromDegrees(100);
    EXPECT_EQ(s3, s3);
    EXPECT_NE(s0, s3);
    EXPECT_NE(s1, s3);
    EXPECT_NE(s2, s3);

    s4.offset = 1.2f;
    EXPECT_EQ(s4, s4);
    EXPECT_NE(s0, s4);
    EXPECT_NE(s1, s4);
    EXPECT_NE(s2, s4);
    EXPECT_NE(s3, s4);
}
