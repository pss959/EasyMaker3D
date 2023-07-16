#include "Tests/Testing.h"
#include "Util/Alarm.h"

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
