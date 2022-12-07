#include <chrono>
#include <thread>

#include "Tests/Testing.h"
#include "Util/Delay.h"
#include "Util/UTime.h"

TEST(Delay, RunDelayed) {
    Util::ResetDelay();
    bool flag = false;
    Util::RunDelayed(.1f, [&flag](){ flag = true; });
    EXPECT_FALSE(flag);
    EXPECT_TRUE(Util::IsAnyDelaying());
    // Add some slack to the sleep time for safety.
    std::this_thread::sleep_for(std::chrono::milliseconds(110));
    EXPECT_TRUE(flag);
    EXPECT_FALSE(Util::IsAnyDelaying());
}

TEST(Delay, CancelDelayed) {
    Util::ResetDelay();
    int counter = 0;
    const int id0 = Util::RunDelayed(8, [&counter](){ ++counter; });
    const int id1 = Util::RunDelayed(4, [&counter](){ ++counter; });
    EXPECT_EQ(0, counter);
    EXPECT_TRUE(Util::IsAnyDelaying());

    Util::CancelDelayed(id1);
    EXPECT_EQ(0, counter);
    EXPECT_TRUE(Util::IsAnyDelaying());

    Util::CancelDelayed(id0);
    EXPECT_EQ(0, counter);
    EXPECT_FALSE(Util::IsAnyDelaying());
}
