#include "Hand.h"
#include "Testing.h"

TEST(Hand, Values) {
    EXPECT_NE(Hand::kLeft, Hand::kRight);
}
