//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Point3fMap.h"
#include "Tests/Testing.h"

TEST(Point3fMapTest, Exact) {
    Point3fMap pmap(0);  // No precision rounding.

    const Point3f p0(1, 2, 3);
    const Point3f p1(-1, -2, -3);
    const Vector3f eps(.000001f, 0, 0);

    Point3f pos;

    EXPECT_EQ(0U, pmap.Add(p0, &pos));
    EXPECT_EQ(p0, pos);
    EXPECT_EQ(1U, pmap.GetCount());
    EXPECT_TRUE(pmap.Contains(p0));
    EXPECT_FALSE(pmap.Contains(p0 + eps));
    EXPECT_EQ(1U, pmap.GetPoints().size());

    EXPECT_EQ(1U, pmap.Add(p1, &pos));
    EXPECT_EQ(p1, pos);
    EXPECT_EQ(2U, pmap.GetCount());
    EXPECT_TRUE(pmap.Contains(p0));
    EXPECT_TRUE(pmap.Contains(p1));
    EXPECT_FALSE(pmap.Contains(p0 + eps));
    EXPECT_FALSE(pmap.Contains(p1 + eps));
    EXPECT_EQ(2U, pmap.GetPoints().size());
}

TEST(Point3fMapTest, Rounding) {
    Point3fMap pmap(.0001f);  // Round to this value.

    const Point3f p0(1, 2, 3);
    const Point3f p1(-1, -2, -3);
    const Vector3f eps(.00004f, 0, 0);  // Within rounding precision.

    Point3f pos;

    EXPECT_EQ(0U, pmap.Add(p0, &pos));
    EXPECT_EQ(p0, pos);
    EXPECT_EQ(1U, pmap.GetCount());
    EXPECT_TRUE(pmap.Contains(p0));
    EXPECT_TRUE(pmap.Contains(p0 + eps));
    EXPECT_EQ(1U, pmap.GetPoints().size());

    EXPECT_EQ(0U, pmap.Add(p0 + eps));

    EXPECT_EQ(1U, pmap.Add(p1, &pos));
    EXPECT_EQ(p1, pos);
    EXPECT_EQ(2U, pmap.GetCount());
    EXPECT_TRUE(pmap.Contains(p0));
    EXPECT_TRUE(pmap.Contains(p1));
    EXPECT_TRUE(pmap.Contains(p0 + eps));
    EXPECT_TRUE(pmap.Contains(p1 + eps));
    EXPECT_EQ(2U, pmap.GetPoints().size());

    EXPECT_EQ(1U, pmap.Add(p1 + eps));
}
