#include "Enums/Hand.h"
#include "Tests/Testing.h"

TEST(HandTest, Values) {
    EXPECT_NE(Hand::kLeft, Hand::kRight);
}
