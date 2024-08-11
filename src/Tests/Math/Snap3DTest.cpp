//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Bounds.h"
#include "Math/Snap3D.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class Snap3DTest : public TestBase {};

TEST_F(Snap3DTest, SnapToBounds) {
    const Bounds bounds(Point3f(-1, -2, -3), Point3f(4, 5, 6));

    const Vector3f tolerance(.5f, .5f, .5f);
    Point3f p;
    Dimensionality dims;

    p.Set(0, 0, 0);
    dims = Snap3D::SnapToBounds(bounds, p, tolerance);
    EXPECT_EQ("", dims.GetAsString());
    EXPECT_EQ(Point3f(0, 0, 0), p);

    p.Set(0, 0, 5.8f);
    dims = Snap3D::SnapToBounds(bounds, p, tolerance);
    EXPECT_EQ("Z", dims.GetAsString());
    EXPECT_EQ(Point3f(0, 0, 6), p);

    p.Set(-.8f, 0, 6.2f);
    dims = Snap3D::SnapToBounds(bounds, p, tolerance);
    EXPECT_EQ("XZ", dims.GetAsString());
    EXPECT_EQ(Point3f(-1, 0, 6), p);

    p.Set(-.8f, 1.6f, 6.2f);
    dims = Snap3D::SnapToBounds(bounds, p, tolerance);
    EXPECT_EQ("XYZ", dims.GetAsString());
    EXPECT_EQ(Point3f(-1, 1.5f, 6), p);
}
