#include "Debug/Timer.h"  // To help diagnose timing issues.
#include "Math/MeshBuilding.h"
#include "Math/MeshSlicing.h"
#include "Math/Spin.h"
#include "Models/CylinderModel.h"
#include "Models/TwistedModel.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

// These tests can be used in conjunction with valgrind/callgrind to
// investigate slow parts of the application. These are filtered out by default
// in TestMain.cpp and can be run by setting the gtest filter to include them.
class TimingTest : public SceneTestBase {
};

TEST_F(TimingTest, ChangeCylinder) {
    // Test changing cylinder radius and validating its mesh. This used to be
    // very slow before setting up to always build all CGAL-related code with
    // optimized flags.
    auto cyl = Model::CreateModel<CylinderModel>();
    std::string reason;
    for (int i = 0; i < 100; ++i) {
        cyl->SetTopRadius(i * .001f);
        EXPECT_TRUE(cyl->IsMeshValid(reason));
    }
}

TEST_F(TimingTest, Slicing) {
    // Slice a box with several slicing planes. Used this to compare the
    // CGAL-based version with the custom slicing version.
    const TriMesh box = BuildBoxMesh(Vector3f(10, 10, 10));

    // Custom slicer.
    Debug::Timer timer("Custom Slicing");
    for (int i = 0; i < 100; ++i) {
        const SlicedMesh sm = SliceMesh(box, Dim::kY, 20);
        EXPECT_EQ(160U,  sm.mesh.points.size());
    }
    timer.Report();
}

TEST_F(TimingTest, Twist) {
    Debug::Timer timer("Twist timing test");

    // Test changing the twist angle for a high-complexity CylinderModel
    // converted to a TwistedModel.
    auto cyl = Model::CreateModel<CylinderModel>();
    cyl->SetComplexity(.8f);
    timer.AddTimePoint("Cylinder created");

    auto tw  = Model::CreateModel<TwistedModel>();
    tw->SetOperandModel(cyl);
    timer.AddTimePoint("TwistedModel operand model set");

    tw->SetComplexity(.8f);
    timer.AddTimePoint("TwistedModel complexity set");

    std::string reason;
    EXPECT_TRUE(tw->IsMeshValid(reason));
    timer.AddTimePoint("TwistedModel first mesh access");

    timer.AddTimePoint("Entering loop");
    for (int i = 0; i < 10; ++i) {
        Spin spin;
        spin.angle = Anglef::FromDegrees(i + 1);
        tw->SetSpin(spin);
        EXPECT_TRUE(tw->IsMeshValid(reason));
    }
    timer.AddTimePoint("Loop done");
    timer.Report();
}
