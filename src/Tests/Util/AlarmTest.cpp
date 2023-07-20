#include "Tests/Testing.h"
#include "Util/Alarm.h"
#include "Util/Delay.h"

TEST(AlarmTest, Running) {
    Alarm alarm;
    EXPECT_FALSE(alarm.IsRunning());
    EXPECT_FALSE(alarm.IsFinished());
    // Run with long duration.
    alarm.Start(100);
    EXPECT_TRUE(alarm.IsRunning());
    EXPECT_FALSE(alarm.IsFinished());
    alarm.Stop();
    EXPECT_FALSE(alarm.IsRunning());
    EXPECT_FALSE(alarm.IsFinished());

    // Run with very short duration.
    alarm.Start(1.e-15);
    EXPECT_TRUE(alarm.IsFinished());
}

TEST(AlarmTest, TimeOut) {
    Alarm alarm;
    // Run with short duration.
    alarm.Start(.00001f);
    // Wait and make sure the alarm is no longer running.
    Util::DelayThread(.0001f);
    EXPECT_TRUE(alarm.IsFinished());
    EXPECT_FALSE(alarm.IsRunning());
}
