#include "Managers/AnimationManager.h"
#include "Tests/Testing.h"
#include "Util/Delay.h"

TEST(AnimationManagerTest, Animate) {
    AnimationManager am;

    // Use the default frame increment behavior.
    am.SetFrameIncrement(0);

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

TEST(AnimationManagerTest, AnimateFrames) {
    AnimationManager am;

    // Set the amount to increment time per frame.
    am.SetFrameIncrement(.01f);

    // Animate for this duration.
    const float kDuration = .05f;

    int frame = 0;
    auto func = [&](float t){
        ++frame;
        EXPECT_EQ(frame * .01f, t);
        return t < kDuration;
    };

    am.StartAnimation(func);

    do {
        am.ProcessUpdate();
    } while (am.IsAnimating());
}
