#include "Math/Animation.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class AnimationTest : public TestBase {};

TEST_F(AnimationTest, Dampen) {
    EXPECT_EQ(0,    Dampen(0));
    EXPECT_GT(.25f, Dampen(.25f));
    EXPECT_EQ(.5f,  Dampen(.5f));
    EXPECT_LT(.75f, Dampen(.75f));
    EXPECT_EQ(1,    Dampen(1));
}
