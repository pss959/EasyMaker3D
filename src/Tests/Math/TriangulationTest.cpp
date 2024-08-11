//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include <ion/math/vectorutils.h>

#include "Math/Triangulation.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class TriangulationTest : public TestBase {
  protected:
    /// Verifies that all result triangles are counterclockwise.
    static void ValidateOrientation(const std::vector<Point2f> &points,
                                    const std::vector<GIndex> &indices) {
        EXPECT_EQ(0U, indices.size() % 3U);
        const size_t tri_count = indices.size() / 3U;
        for (size_t i = 0; i < tri_count; ++i) {
            const size_t i0 = indices[3 * i + 0];
            const size_t i1 = indices[3 * i + 1];
            const size_t i2 = indices[3 * i + 2];
            EXPECT_TRUE(IsCCW_(points[i0], points[i1], points[i2]));
        }
    }

  private:
    /// Returns true if the triangle formed by three points is
    /// counterclockwise.
    static bool IsCCW_(const Point2f &p0, const Point2f &p1, const Point2f &p2) {
        // Use 2D cross product; the sign indicates whether the angle is
        // positive (CCW) or negative (CW).
        const Vector2f v0 = p1 - p0;
        const Vector2f v1 = p2 - p0;
        return ion::math::Cross(v0, v1) > 0.f;
    }
};

TEST_F(TriangulationTest, Triangle) {
    // Very simple test.
    const std::vector<Point2f> points{
        Point2f(0, 0),
        Point2f(0, 1),
        Point2f(1, 0),
    };
    const std::vector<GIndex> indices = TriangulatePolygon(Polygon(points));
    EXPECT_EQ(3U, indices.size());
    ValidateOrientation(points, indices);
}

TEST_F(TriangulationTest, Rect) {
    const std::vector<Point2f> points{
        Point2f(-3, -5),
        Point2f(-3,  5),
        Point2f( 3,  5),
        Point2f( 3, -5),
    };
    const std::vector<GIndex> indices = TriangulatePolygon(Polygon(points));
    EXPECT_EQ(6U, indices.size());  // 2 triangles.
    ValidateOrientation(points, indices);
}

TEST_F(TriangulationTest, RectWithHole) {
    const std::vector<Point2f> points{
        // Outer border.
        Point2f(-3, -5),
        Point2f(-3,  5),
        Point2f( 3,  5),
        Point2f( 3, -5),
        // Hole.
        Point2f(-1, -3),
        Point2f( 1, -3),
        Point2f( 1,  3),
        Point2f(-1,  3),
    };
    const std::vector<size_t> border_counts{ 4, 4 };
    const std::vector<GIndex> indices =
        TriangulatePolygon(Polygon(points, border_counts));
    EXPECT_EQ(24U, indices.size());  // 8 triangles.
    ValidateOrientation(points, indices);
}

TEST_F(TriangulationTest, LowerCaseT) {
    // Vertices forming a lower-case Arial 't'. This resulted in T-junctions
    // with the libtess2-based triangulator.
    const std::vector<Point2f> points{
        Point2f(0.5217692f, -1.530753f),
        Point2f(0.4257083f, -1.535591f),
        Point2f(0.3279198f, -1.526607f),
        Point2f(0.2557014f, -1.499654f),
        Point2f(0.1962681f, -1.463718f),
        Point2f(0.1700069f, -1.402902f),
        Point2f(0.1503109f, -1.304077f),
        Point2f(0.1437457f, -1.129233f),
        Point2f(0.1437457f, 0.5874224f),
        Point2f(0.6440911f, 0.5874224f),
        Point2f(0.6440911f, 0.9744298f),
        Point2f(0.1437457f, 0.9744298f),
        Point2f(0.1437457f, 2.f),
        Point2f(-0.3510712f, 1.701451f),
        Point2f(-0.3510712f, 0.9744298f),
        Point2f(-0.7159641f, 0.9744298f),
        Point2f(-0.7159641f, 0.5874224f),
        Point2f(-0.3510712f, 0.5874224f),
        Point2f(-0.3510712f, -1.101589f),
        Point2f(-0.3486386f, -1.26839f),
        Point2f(-0.3413408f, -1.410532f),
        Point2f(-0.3291776f, -1.528017f),
        Point2f(-0.3121493f, -1.620843f),
        Point2f(-0.2902557f, -1.689012f),
        Point2f(-0.2073255f, -1.815135f),
        Point2f(-0.08016592f, -1.914305f),
        Point2f(0.03286487f, -1.961913f),
        Point2f(0.1729248f, -1.990478f),
        Point2f(0.3400137f, -2.f),
        Point2f(0.5169315f, -1.988943f),
        Point2f(0.7159641f, -1.95577f),
    };
    Polygon poly(points);
    const std::vector<GIndex> indices = TriangulatePolygon(Polygon(points));

    // There should be 29 result triangles.
    EXPECT_EQ(29U * 3U, indices.size());
    ValidateOrientation(points, indices);
}

TEST_F(TriangulationTest, UpperCaseQ) {
    // Vertices forming an upper-case Arial 'Q'. This resulted in missing
    // triangles.
    const std::vector<Point2f> points{
        // Outer border (32 points).
        Point2f(21.34f, 4.27f),
        Point2f(22.42f, 6.38f),
        Point2f(23.07f, 8.78f),
        Point2f(23.28f, 11.47f),
        Point2f(23.13f, 13.66f),
        Point2f(22.69f, 15.70f),
        Point2f(21.95f, 17.59f),
        Point2f(20.31f, 20.00f),
        Point2f(18.06f, 21.80f),
        Point2f(15.36f, 22.94f),
        Point2f(12.34f, 23.31f),
        Point2f(9.35f, 22.95f),
        Point2f(6.66f, 21.86f),
        Point2f(4.42f, 20.07f),
        Point2f(2.75f, 17.63f),
        Point2f(1.99f, 15.70f),
        Point2f(1.53f, 13.65f),
        Point2f(1.38f, 11.45f),
        Point2f(1.53f, 9.26f),
        Point2f(1.98f, 7.22f),
        Point2f(2.74f, 5.33f),
        Point2f(4.40f, 2.91f),
        Point2f(6.64f, 1.11f),
        Point2f(9.32f, -0.02f),
        Point2f(12.28f, -0.39f),
        Point2f(15.23f, -0.04f),
        Point2f(17.91f, 1.03f),
        Point2f(20.36f, -0.63f),
        Point2f(22.83f, -1.78f),
        Point2f(23.72f, 0.33f),
        Point2f(21.86f, 1.20f),
        Point2f(19.83f, 2.45f),

        // Hole (28 points).
        Point2f(12.42f, 4.13f),
        Point2f(13.96f, 3.60f),
        Point2f(15.42f, 2.81f),
        Point2f(13.98f, 2.20f),
        Point2f(12.36f, 2.20f),
        Point2f(10.20f, 2.47f),
        Point2f(8.32f, 3.27f),
        Point2f(6.71f, 4.59f),
        Point2f(5.48f, 6.42f),
        Point2f(4.75f, 8.70f),
        Point2f(4.50f, 11.45f),
        Point2f(4.75f, 14.27f),
        Point2f(5.49f, 16.58f),
        Point2f(6.73f, 18.38f),
        Point2f(8.35f, 19.67f),
        Point2f(10.23f, 20.45f),
        Point2f(12.36f, 20.70f),
        Point2f(14.51f, 20.42f),
        Point2f(16.45f, 19.55f),
        Point2f(18.05f, 18.17f),
        Point2f(19.21f, 16.33f),
        Point2f(19.92f, 14.07f),
        Point2f(20.16f, 11.47f),
        Point2f(19.86f, 8.48f),
        Point2f(18.97f, 6.04f),
        Point2f(17.48f, 4.14f),
        Point2f(15.55f, 5.41f),
        Point2f(13.16f, 6.33f),
    };

    const std::vector<size_t> border_counts{ 32, 28 };
    const std::vector<GIndex> indices =
        TriangulatePolygon(Polygon(points, border_counts));
    EXPECT_EQ(60U * 3U, indices.size());  // 60 triangles.
    ValidateOrientation(points, indices);
}
