#include "Enums/Hand.h"
#include "Tests/Testing.h"

TEST(Hand, Values) {
    EXPECT_NE(Hand::kLeft, Hand::kRight);
}
