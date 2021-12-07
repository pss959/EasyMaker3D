#include <chrono>
#include <thread>

#include "Testing.h"
#include "Util/Time.h"

TEST(Time, Operators) {
    Util::Time t1 = Util::Time::Now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    Util::Time t2 = t1;
    Util::Time t3 = Util::Time::Now();

    EXPECT_TRUE(t1 == t2);
    EXPECT_FALSE(t1 != t2);
    EXPECT_FALSE(t1 < t2);
    EXPECT_FALSE(t1 > t2);
    EXPECT_TRUE(t1 <= t2);
    EXPECT_TRUE(t1 >= t2);

    EXPECT_FALSE(t1 == t3);
    EXPECT_TRUE(t1 != t3);
    EXPECT_TRUE(t1 < t3);
    EXPECT_TRUE(t1 <= t3);
    EXPECT_TRUE(t3 > t1);
    EXPECT_TRUE(t3 >= t1);
}

TEST(Time, SecondsSince) {
    Util::Time t1 = Util::Time::Now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    Util::Time t2 = Util::Time::Now();
    EXPECT_LT(0., t2.SecondsSince(t1));
}

TEST(Time, ToString) {
    // The undefined time differs per platform.
#if defined(ION_PLATFORM_WINDOWS)
    const std::string ts = Util::Time().ToString();
    EXPECT_TRUE(ts == "Fri Dec 31 15:59:59 2173" ||
                ts == "Fri Dec 31 16:00:00 2173");
#else
    EXPECT_EQ("Fri Dec 31 15:59:59 2173", Util::Time().ToString());
#endif
}
