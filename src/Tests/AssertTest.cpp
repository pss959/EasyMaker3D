#include "Tests/Testing.h"
#include "Util/Assert.h"

TEST(Assert, Pass) {
    ASSERT(5 == 5);
    ASSERTM(3 >= 3, "This should not fail");
}

TEST(Assert, Fail) {
    TEST_THROW(ASSERT(4 == 3), AssertException, "4 == 3");
    TEST_THROW(ASSERTM(5 > 6, "Test message"), AssertException, "Test message");
}
