#include "Math/Types.h"
#include "Tests/Testing.h"

TEST(ColorTest, Constructors) {
    EXPECT_EQ(Color(0, 0, 0, 1), Color());
    EXPECT_EQ(Color(0, 0, 0, 1), Color(0, 0, 0));
    EXPECT_EQ(Color(.1, .2, .3, 1), Color(.1, .2, .3));
    EXPECT_EQ(Color(0, 0, 0, 1), Color::Black());
    EXPECT_EQ(Color(1, 1, 1, 1), Color::White());
    EXPECT_EQ(Color(0, 0, 0, 0), Color::Clear());
}

TEST(ColorTest, FromHexString) {
    Color c;

    EXPECT_TRUE(c.FromHexString("#ffffffff"));
    EXPECT_EQ(Color(1, 1, 1, 1), c);

    EXPECT_TRUE(c.FromHexString("#ffffff"));
    EXPECT_EQ(Color(1, 1, 1, 1), c);

    EXPECT_TRUE(c.FromHexString("#00000000"));
    EXPECT_EQ(Color(0, 0, 0, 0), c);

    EXPECT_TRUE(c.FromHexString("#abcdef"));
    EXPECT_TRUE(c.FromHexString("#01abcdef"));
    EXPECT_TRUE(c.FromHexString("#deadbeef"));

    // Errors
    EXPECT_FALSE(c.FromHexString("#0000000"));
    EXPECT_FALSE(c.FromHexString("#00000"));
    EXPECT_FALSE(c.FromHexString("#abcdeg"));
    EXPECT_FALSE(c.FromHexString("#01abcdeg"));
    EXPECT_FALSE(c.FromHexString("#aabbccdd1"));
}
