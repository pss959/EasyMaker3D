#include "Math/Bevel.h"
#include "Math/Beveler.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Math/MeshValidation.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

class BevelerTest : public TestBase {
  protected:
    // Returns a 2-point Bevel with optional scale and max_angle settings.
    static Bevel GetDefaultBevel(float scale = 1, float max_angle = 120) {
        Bevel bevel;
        bevel.max_angle = Anglef::FromDegrees(max_angle);
        bevel.scale     = scale;
        return bevel;
    }

    // Returns a Bevel with the given number of points (3-6) and optional scale
    // and max_angle settings.
    static Bevel GetBevel(size_t np, float scale = 1, float max_angle = 120) {
        static const Point2f pts[4]{
            { .4f, .8f }, { .5f, .8f }, { .8f, .6f }, { .9f, .2f }
        };

        ASSERT(np >= 3U && np <= 6U);
        Bevel bevel;
        bevel.max_angle = Anglef::FromDegrees(max_angle);
        bevel.scale = scale;

        const auto add_pts = [&](const std::vector<size_t> indices){
            for (const auto i: indices)
                bevel.profile.AddPoint(pts[i]);
        };
        if (np == 3)
            add_pts(std::vector<size_t>{0});
        else if (np == 4)
            add_pts(std::vector<size_t>{0, 3});
        else if (np == 5)
            add_pts(std::vector<size_t>{0, 1, 3});
        else if (np == 6)
            add_pts(std::vector<size_t>{0, 1, 2, 3});

        return bevel;
    }

    // Runs a bevel test on the given input TriMesh with the given Bevel. The
    // expected number of resulting points and triangles are supplied.
    void TestBevel(const TriMesh &m, const Bevel &bevel,
                   size_t expected_point_count, size_t expected_tri_count) {
        SCOPED_TRACE(GetTestName() + " " + bevel.ToString());
        const TriMesh rm = Beveler::ApplyBevel(m, bevel);
        ValidateMesh(rm, GetTestName());
        EXPECT_EQ(expected_point_count, rm.points.size());
        EXPECT_EQ(expected_tri_count, rm.GetTriangleCount());
    }
};

TEST_F(BevelerTest, BevelBox) {
    const TriMesh m = BuildBoxMesh(Vector3f(10, 14, 10));
    TestBevel(m, GetDefaultBevel(),  24,  44);
    TestBevel(m, GetBevel(3),        48,  92);
    TestBevel(m, GetBevel(4),        96, 188);
    TestBevel(m, GetBevel(5),       144, 284);
    TestBevel(m, GetBevel(6),       216, 428);
}

TEST_F(BevelerTest, BevelCyl) {
    const TriMesh m = BuildCylinderMesh(5, 5, 20, 7);
    TestBevel(m, GetDefaultBevel(1, 120), 28,  52);
    TestBevel(m, GetBevel(3, 1, 120),     42,  80);
    TestBevel(m, GetBevel(4, 1, 120),     56, 108);
    TestBevel(m, GetBevel(5, 1, 120),     70, 136);
    TestBevel(m, GetBevel(6, 1, 120),     84, 164);
}

TEST_F(BevelerTest, BevelCyl2) {
    // Cylinder with larger number of sides
    const TriMesh m = BuildCylinderMesh(5, 5, 20, 20);
    TestBevel(m, GetDefaultBevel(1, 120), 80, 156);
    TestBevel(m, GetBevel(3, 1, 120),    120, 236);
    TestBevel(m, GetBevel(4, 1, 120),    160, 316);
    TestBevel(m, GetBevel(5, 1, 120),    200, 396);
    TestBevel(m, GetBevel(6, 1, 120),    240, 476);
}

TEST_F(BevelerTest, BevelHole) {
    // Simple geometry with a hole.
    const TriMesh m = LoadTriMesh("hole.stl");
    TestBevel(m, GetDefaultBevel(), 48, 96);
}

TEST_F(BevelerTest, BevelTextO) {
    const TriMesh m = LoadTriMesh("O.stl");
    TestBevel(m, GetDefaultBevel(.25f), 80, 160);

    // Repeat with a small scale - this used to create an unclosed mesh.
    TestBevel(m, GetDefaultBevel(.02f), 80, 160);
}

TEST_F(BevelerTest, BevelTextA) {
    const TriMesh m = LoadTriMesh("A.stl");
    // Have to use a very small scale for a valid mesh.
    TestBevel(m, GetDefaultBevel(.05f), 78, 156);
}

TEST_F(BevelerTest, BevelTetrahedron) {
    const TriMesh m = BuildTetrahedronMesh(10);
    TestBevel(m, GetDefaultBevel(),  12,  20);
    TestBevel(m, GetBevel(3),        24,  44);
    TestBevel(m, GetBevel(4),        48,  92);
    TestBevel(m, GetBevel(5),        72, 140);
    TestBevel(m, GetBevel(6),       108, 212);
}

TEST_F(BevelerTest, BevelPyramid) {
    // Pyramid has 4 faces adjacent to the apex vertex.
    const TriMesh m = BuildCylinderMesh(0, 10, 20, 4);
    TestBevel(m, GetDefaultBevel(),  16,  28);
    TestBevel(m, GetBevel(3),        32,  60);
    TestBevel(m, GetBevel(4),        64, 124);
    TestBevel(m, GetBevel(5),        96, 188);
    TestBevel(m, GetBevel(6),       144, 284);
}

TEST_F(BevelerTest, BevelTwist2) {
    // 2-layer model with a twist in it.
    const TriMesh m = LoadTriMesh("Twist2.stl");
    TestBevel(m, GetDefaultBevel(.2f), 40, 76);
}

TEST_F(BevelerTest, BevelTwist3) {
    // 3-layer model with a twist in it.
    const TriMesh m = LoadTriMesh("Twist3.stl");
    TestBevel(m, GetDefaultBevel(.2f), 56, 108);
}

TEST_F(BevelerTest, BevelBend) {
    // Extruded pentagon model with a bend.
    const TriMesh m = LoadTriMesh("Bend.stl");
    TestBevel(m, GetDefaultBevel(.2f), 46, 88);
}
