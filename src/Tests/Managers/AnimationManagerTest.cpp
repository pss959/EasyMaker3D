#include "Managers/AnimationManager.h"
#include "Tests/Testing.h"
#include "Util/Delay.h"

TEST(AnimationManagerTest, Animate) {
    AnimationManager am;

    // Animate for this duration.
    const float kDuration = .001f;

    // Delay this much between updates.
    const float kDelay = .1f * kDuration;

    float prev_t = 0;
    auto func = [&](float t){
        EXPECT_LT(prev_t, t + kDelay);
        prev_t = t;
        return t < kDuration;
    };

    am.StartAnimation(func);

    do {
        Util::DelayThread(kDelay);
        am.ProcessUpdate();
    } while (am.IsAnimating());

    EXPECT_LE(kDuration, prev_t);
}
