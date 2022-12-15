#include "Math/Bevel.h"
#include "Math/Beveler.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Math/MeshValidation.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

#include "Debug/Dump3dv.h" // XXXX

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
    TestBevel(m, GetBevel(3, 1, 120),     56, 108);
    TestBevel(m, GetBevel(4, 1, 120),    126, 248);
    TestBevel(m, GetBevel(5, 1, 120),    196, 388);
    TestBevel(m, GetBevel(6, 1, 120),    308, 612);
}

TEST_F(BevelerTest, BevelCyl2) {
    // Cylinder with larger number of sides
    const TriMesh m = BuildCylinderMesh(5, 5, 20, 20);
    TestBevel(m, GetDefaultBevel(1, 120), 80,  156);
    TestBevel(m, GetBevel(3, 1, 120),    160,  316);
    TestBevel(m, GetBevel(4, 1, 120),    360,  716);
    TestBevel(m, GetBevel(5, 1, 120),    560, 1116);
    TestBevel(m, GetBevel(6, 1, 120),    880, 1756);
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
    TestBevel(m, GetDefaultBevel(.2f), 0, 0);
}

TEST_F(BevelerTest, TMPA) {
    const TriMesh m = BuildBoxMesh(Vector3f(10, 14, 10));
    // EnableKLog("l"); // XXXX
    //const Bevel bevel = GetDefaultBevel(1);
    const Bevel bevel = GetBevel(4);
    const TriMesh rm = Beveler::ApplyBevel(m, bevel);
    std::cerr << "XXXX VALIDITY CODE = "
              << Util::EnumName(ValidateTriMesh(rm)) << "\n";
}

TEST_F(BevelerTest, TMPB) {
    const TriMesh m = LoadTriMesh("Bend.stl");
    // EnableKLog("l"); // XXXX
    const TriMesh rm = Beveler::ApplyBevel(m, GetDefaultBevel(.2f));
    std::cerr << "XXXX VALIDITY CODE = "
              << Util::EnumName(ValidateTriMesh(rm)) << "\n";

#if 1 // XXXX
    {  // XXXX
        Debug::Dump3dv dump("/tmp/RTMESH.3dv", "Result Beveled TriMesh");
        dump.SetLabelFontSize(12);
        Debug::Dump3dv::LabelFlags label_flags;
        label_flags.Set(Debug::Dump3dv::LabelFlag::kVertexLabels);
        //label_flags.Set(Debug::Dump3dv::LabelFlag::kEdgeLabels);
        //label_flags.Set(Debug::Dump3dv::LabelFlag::kFaceLabels);
        dump.SetLabelFlags(label_flags);
        dump.AddTriMesh(rm);
    }
#endif
    /* XXXX
       In UMESH:
         Edge from V10/V32/V38/V59 to V12/V13 is bad. (V10 to V12)
         Edge from V10/V32/V38/V59 to V30/V35 is bad. (V32 to V30)
         Face F2: V12 V10 V11
         Face F8: V32 V30 V31

       Border edges in RTMESH.3dv:
       # V11 (3 0 -4) to V10 (0.3764 0.9604 -0.0294)
       # V10 (0.3764 0.9604 -0.0294) to V13 (2.8919 0.0396 -3.8364)
       # V16 (0.3121 0.9812 -0.0601) to V15 (1.2 0.6 -4)
       # V18 (1.1562 0.6188 -3.8058) to V16 (0.3121 0.9812 -0.0601)
     */
}

TEST_F(BevelerTest, BevelHelix) {
    // A pretty complex STL model.
    const TriMesh m = LoadTriMesh("double_helix.stl");
    // Have to use a small scale for a valid mesh.
    TestBevel(m, GetDefaultBevel(.4f), 1265, 2172);
}
