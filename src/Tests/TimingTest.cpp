#include "Debug/Timer.h"  // To help diagnose timing issues.
#include "Math/Twist.h"
#include "Models/CylinderModel.h"
#include "Models/TwistedModel.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

// This test can be used in conjunction with valgrind/callgrind to investigate
// slow parts of the application. These are generally #if-ed out so that they
// do not slow down regular testing.
class TimingTest : public SceneTestBase {
};

#if 0
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
        Twist twist;
        twist.angle = Anglef::FromDegrees(i + 1);
        tw->SetTwist(twist);
        EXPECT_TRUE(tw->IsMeshValid(reason));
    }
    timer.AddTimePoint("Loop done");
    timer.Report();
}
#endif
