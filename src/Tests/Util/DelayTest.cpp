//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include <chrono>
#include <thread>

#include "Tests/Testing.h"
#include "Util/Delay.h"
#include "Util/UTime.h"

TEST(DelayTest, DelayThread) {
    const float delay_secs = .02f;
    const auto t0 = UTime::Now();
    Util::DelayThread(delay_secs);
    const auto t1 = UTime::Now();
    EXPECT_LE(delay_secs,         t1.SecondsSince(t0));
    EXPECT_GE(delay_secs + .001f, t1.SecondsSince(t0));
}

TEST(DelayTest, RunDelayed) {
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

TEST(DelayTest, CancelDelayed) {
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
