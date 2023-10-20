#include "SG/ShadowPass.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class ShadowPassTest : public SceneTestBase {};

TEST_F(ShadowPassTest, ShadowsEnabled) {
    auto sp = CreateObject<SG::ShadowPass>();

    EXPECT_NOT_NULL(sp);
    EXPECT_TRUE(sp->AreShadowsEnabled());
    sp->SetShadowsEnabled(false);
    EXPECT_FALSE(sp->AreShadowsEnabled());
}
