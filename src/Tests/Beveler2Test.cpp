#include "Debug/Dump3dv.h"
#include "Math/Bevel.h"
#include "Math/Beveler.h" // XXXX
#include "Math/Beveler2.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Math/MeshValidation.h"
#include "TestBase.h"
#include "Testing.h"

class Beveler2Test : public TestBase {
  protected:
    // Beveler2Test() { EnableKLog("l"); }  // XXXX For debugging.

    // Returns a default Bevel.
    static Bevel GetDefaultBevel(float scale = 1) {
        Bevel bevel;
        // XXXX Bevel all edges for now...
        bevel.max_angle = Anglef::FromDegrees(180);
        bevel.scale     = scale;
        return bevel;
    }

    // Returns a Bevel with the given number of point and optional scale.
    static Bevel GetBevel(size_t np, float scale = 1) {
        static const Point2f pts[4]{
            { .4f, .8f }, { .5f, .8f }, { .8f, .6f }, { .9f, .2f }
        };

        ASSERT(np <= 6U);
        Bevel bevel;
        // XXXX Bevel all edges for now...
        bevel.max_angle = Anglef::FromDegrees(180);
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
        const TriMesh rm = Beveler2::ApplyBevel(m, bevel);
        ValidateMesh(rm, GetTestName());
        EXPECT_EQ(expected_point_count, rm.points.size());
        EXPECT_EQ(expected_tri_count, rm.GetTriangleCount());
    }
};

TEST_F(Beveler2Test, BevelBox) {
    const TriMesh m = BuildBoxMesh(Vector3f(10, 14, 10));
    TestBevel(m, GetDefaultBevel(),  24,  44);
    TestBevel(m, GetBevel(3),        48,  92);
    TestBevel(m, GetBevel(4),        96, 188);
    TestBevel(m, GetBevel(5),       144, 284);
    TestBevel(m, GetBevel(6),       216, 428);
}

TEST_F(Beveler2Test, BevelCyl) {
    const TriMesh m = BuildCylinderMesh(5, 5, 20, 7);
    TestBevel(m, GetDefaultBevel(),  42,  80);
    TestBevel(m, GetBevel(3),        84, 164);
    TestBevel(m, GetBevel(4),       168, 332);
    TestBevel(m, GetBevel(5),       252, 500);
    TestBevel(m, GetBevel(6),       378, 752);
}

TEST_F(Beveler2Test, BevelTextO) {
    const TriMesh m = LoadTriMesh("O.stl");
    TestBevel(m, GetDefaultBevel(.25f), 120, 240);
}

TEST_F(Beveler2Test, BevelTextA) {
    const TriMesh m = LoadTriMesh("A.stl");
    // Have to use a very small scale for a valid mesh.
    TestBevel(m, GetDefaultBevel(.05f), 84, 168);
}

TEST_F(Beveler2Test, BevelTetrahedron) {
    const TriMesh m = BuildTetrahedronMesh(10);
    TestBevel(m, GetDefaultBevel(),  12,  20);
    TestBevel(m, GetBevel(3),        24,  44);
    TestBevel(m, GetBevel(4),        48,  92);
    TestBevel(m, GetBevel(5),        72, 140);
    TestBevel(m, GetBevel(6),       108, 212);
}

TEST_F(Beveler2Test, BevelPyramid) {
    // Pyramid has 4 faces adjacent to the apex vertex.
    const TriMesh m = BuildCylinderMesh(0, 10, 20, 4);
    TestBevel(m, GetDefaultBevel(),  16,  28);
    TestBevel(m, GetBevel(3),        32,  60);
    TestBevel(m, GetBevel(4),        64, 124);
    TestBevel(m, GetBevel(5),        96, 188);
    TestBevel(m, GetBevel(6),       144, 284);
}

TEST_F(Beveler2Test, BevelHelix) {
    // A pretty complex STL model.
    const TriMesh m = LoadTriMesh("double_helix.stl");
    // Have to use a small scale for a valid mesh.
    TestBevel(m, GetDefaultBevel(.2f), 2056, 4112);
}
