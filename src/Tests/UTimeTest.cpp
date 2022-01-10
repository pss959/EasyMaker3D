#include <chrono>
#include <thread>

#include "Testing.h"
#include "Util/UTime.h"

TEST(UTime, Operators) {
    UTime t1 = UTime::Now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    UTime t2 = t1;
    UTime t3 = UTime::Now();

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

TEST(UTime, SecondsSince) {
    UTime t1 = UTime::Now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    UTime t2 = UTime::Now();
    EXPECT_LT(0., t2.SecondsSince(t1));
}

TEST(UTime, ToString) {
    // The undefined time differs per platform.
#if defined(ION_PLATFORM_WINDOWS)
    const std::string ts = UTime().ToString();
    EXPECT_TRUE(ts == "Fri Dec 31 15:59:59 2173" ||
                ts == "Fri Dec 31 16:00:00 2173");
#else
    EXPECT_EQ("Fri Dec 31 15:59:59 2173", UTime().ToString());
#endif
}
