//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Color.h"
#include "Math/Types.h"
#include "Tests/Testing.h"

TEST(ColorTest, Constructors) {
    EXPECT_EQ(Color(0, 0, 0, 1), Color());
    EXPECT_EQ(Color(0, 0, 0, 1), Color(0, 0, 0));
    EXPECT_EQ(Color(.1f, .2f, .3f, 1), Color(.1f, .2f, .3f));
    EXPECT_EQ(Color(.1f, .2f, .3f, .4), Color(Vector4f(.1f, .2f, .3f, .4f)));
    EXPECT_EQ(Color(0, 0, 0, 1), Color::Black());
    EXPECT_EQ(Color(1, 1, 1, 1), Color::White());
    EXPECT_EQ(Color(0, 0, 0, 0), Color::Clear());

    EXPECT_EQ("#ffffffff", Color::White().ToHexString());
    EXPECT_EQ("#000000ff", Color::Black().ToString());
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
