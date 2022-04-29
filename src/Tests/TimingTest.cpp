#include "Models/CylinderModel.h"
#include "SceneTestBase.h"
#include "Testing.h"

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
#endif
