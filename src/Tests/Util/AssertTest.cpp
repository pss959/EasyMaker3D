#include "Tests/Testing2.h"
#include "Util/Assert.h"

TEST(AssertTest, Pass) {
    ASSERT(5 == 5);
    ASSERTM(3 >= 3, "This should not fail");
}

TEST(AssertTest, Fail) {
    TEST_ASSERT(ASSERT(4 == 3), "4 == 3");
    TEST_ASSERT(ASSERTM(5 > 6, "Test message"), "Test message");
}
