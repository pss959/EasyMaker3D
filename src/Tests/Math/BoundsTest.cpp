//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Bounds.h"
#include "Math/Types.h"
#include "Tests/Testing.h"

TEST(BoundsTest, Constructors) {
    // Bounds from min/max.
    const Point3f min(-2, -3, 5);
    const Point3f max(6, 9, 11);
    Bounds mmb(min, max);
    EXPECT_EQ(min,                mmb.GetMinPoint());
    EXPECT_EQ(max,                mmb.GetMaxPoint());
    EXPECT_EQ(Point3f(2, 3, 8),   mmb.GetCenter());
    EXPECT_EQ(Vector3f(8, 12, 6), mmb.GetSize());

    // Bounds from Range3f.
    Bounds rb(Range3f(min, max));
    EXPECT_EQ(mmb, rb);

    // Bounds from size.
    const Vector3f sz(8, 10, 12);
    Bounds szb(sz);
    EXPECT_EQ(Point3f(-4, -5, -6), szb.GetMinPoint());
    EXPECT_EQ(Point3f(4, 5, 6),    szb.GetMaxPoint());
    EXPECT_EQ(Point3f(0, 0, 0),    szb.GetCenter());
    EXPECT_EQ(sz,                  szb.GetSize());
}

TEST(BoundsTest, Faces) {
    const Point3f min(-2, -3, 5);
    const Point3f max(6, 9, 11);
    Bounds b(min, max);

    EXPECT_EQ(Point3f(-2, 3, 8),  b.GetFaceCenter(Bounds::Face::kLeft));
    EXPECT_EQ(Point3f(6,  3, 8),  b.GetFaceCenter(Bounds::Face::kRight));
    EXPECT_EQ(Point3f(2, -3, 8),  b.GetFaceCenter(Bounds::Face::kBottom));
    EXPECT_EQ(Point3f(2,  9, 8),  b.GetFaceCenter(Bounds::Face::kTop));
    EXPECT_EQ(Point3f(2,  3, 5),  b.GetFaceCenter(Bounds::Face::kBack));
    EXPECT_EQ(Point3f(2,  3, 11), b.GetFaceCenter(Bounds::Face::kFront));

    EXPECT_EQ(0, Bounds::GetFaceDim(Bounds::Face::kLeft));
    EXPECT_EQ(0, Bounds::GetFaceDim(Bounds::Face::kRight));
    EXPECT_EQ(1, Bounds::GetFaceDim(Bounds::Face::kBottom));
    EXPECT_EQ(1, Bounds::GetFaceDim(Bounds::Face::kTop));
    EXPECT_EQ(2, Bounds::GetFaceDim(Bounds::Face::kBack));
    EXPECT_EQ(2, Bounds::GetFaceDim(Bounds::Face::kFront));

    EXPECT_FALSE(Bounds::IsFaceMax(Bounds::Face::kLeft));
    EXPECT_TRUE(Bounds::IsFaceMax(Bounds::Face::kRight));
    EXPECT_FALSE(Bounds::IsFaceMax(Bounds::Face::kBottom));
    EXPECT_TRUE(Bounds::IsFaceMax(Bounds::Face::kTop));
    EXPECT_FALSE(Bounds::IsFaceMax(Bounds::Face::kBack));
    EXPECT_TRUE(Bounds::IsFaceMax(Bounds::Face::kFront));

    EXPECT_EQ(Bounds::Face::kLeft,   Bounds::GetFace(0, false));
    EXPECT_EQ(Bounds::Face::kRight,  Bounds::GetFace(0, true));
    EXPECT_EQ(Bounds::Face::kBottom, Bounds::GetFace(1, false));
    EXPECT_EQ(Bounds::Face::kTop,    Bounds::GetFace(1, true));
    EXPECT_EQ(Bounds::Face::kBack,   Bounds::GetFace(2, false));
    EXPECT_EQ(Bounds::Face::kFront,  Bounds::GetFace(2, true));

    EXPECT_EQ(-Vector3f::AxisX(), Bounds::GetFaceNormal(Bounds::Face::kLeft));
    EXPECT_EQ( Vector3f::AxisX(), Bounds::GetFaceNormal(Bounds::Face::kRight));
    EXPECT_EQ(-Vector3f::AxisY(), Bounds::GetFaceNormal(Bounds::Face::kBottom));
    EXPECT_EQ( Vector3f::AxisY(), Bounds::GetFaceNormal(Bounds::Face::kTop));
    EXPECT_EQ(-Vector3f::AxisZ(), Bounds::GetFaceNormal(Bounds::Face::kBack));
    EXPECT_EQ( Vector3f::AxisZ(), Bounds::GetFaceNormal(Bounds::Face::kFront));
}

TEST(BoundsTest, GetFaceForPoint) {
    const Point3f min(-4, -4, -4);
    const Point3f max(4, 4, 4);
    const Bounds b(min, max);

    EXPECT_EQ(Bounds::Face::kLeft,   b.GetFaceForPoint(Point3f(-4,  1,  3)));
    EXPECT_EQ(Bounds::Face::kRight,  b.GetFaceForPoint(Point3f( 4, -1, -3)));
    EXPECT_EQ(Bounds::Face::kBottom, b.GetFaceForPoint(Point3f( 1, -4,  3)));
    EXPECT_EQ(Bounds::Face::kTop,    b.GetFaceForPoint(Point3f(-1,  4, -3)));
    EXPECT_EQ(Bounds::Face::kBack,   b.GetFaceForPoint(Point3f(-3,  1, -4)));
    EXPECT_EQ(Bounds::Face::kFront,  b.GetFaceForPoint(Point3f( 3, -1,  4)));

    // A point not on the bounds always returns kFront.
    EXPECT_EQ(Bounds::Face::kFront,  b.GetFaceForPoint(Point3f(10, 20, -30)));
}

TEST(BoundsTest, Corners) {
    const Point3f min(-2, -3, 5);
    const Point3f max(6, 9, 11);
    const Bounds b(min, max);

    Point3f corners[8];
    b.GetCorners(corners);

    EXPECT_EQ(Point3f(min[0], min[1], min[2]), corners[0]);
    EXPECT_EQ(Point3f(min[0], min[1], max[2]), corners[1]);
    EXPECT_EQ(Point3f(min[0], max[1], min[2]), corners[2]);
    EXPECT_EQ(Point3f(min[0], max[1], max[2]), corners[3]);
    EXPECT_EQ(Point3f(max[0], min[1], min[2]), corners[4]);
    EXPECT_EQ(Point3f(max[0], min[1], max[2]), corners[5]);
    EXPECT_EQ(Point3f(max[0], max[1], min[2]), corners[6]);
    EXPECT_EQ(Point3f(max[0], max[1], max[2]), corners[7]);
}
