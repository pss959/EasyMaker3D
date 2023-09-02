#include <ion/math/transformutils.h>

#include "Math/TextUtils.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"
#include "Util/FilePath.h"
#include "Util/General.h"
#include "Util/Tuning.h"

/// \ingroup Tests
class TextUtilsTest : public TestBase {};

TEST_F(TextUtilsTest, GetAvailableFontNames) {
    const auto names = GetAvailableFontNames();
    EXPECT_FALSE(names.empty());
    EXPECT_TRUE(Util::Contains(names, "Arial-Bold"));
    EXPECT_TRUE(Util::Contains(names, "Arial-Regular"));
    EXPECT_TRUE(Util::Contains(names, "Verdana-Regular"));
}

TEST_F(TextUtilsTest, IsValidFontName) {
    EXPECT_TRUE(IsValidFontName("Arial-Regular"));
    EXPECT_TRUE(IsValidFontName("Verdana-Bold"));
    EXPECT_FALSE(IsValidFontName("No-Such-Font"));
}

TEST_F(TextUtilsTest, IsValidStringForFont) {
    // Note that this uses the FakeFontSystem rules.
    auto test_good = [](const Str &str){
        Str reason;
        EXPECT_TRUE(IsValidStringForFont("Arial-Regular", str, reason));
        EXPECT_TRUE(reason.empty());
    };
    auto test_bad = [](const Str &str, const Str &exp_reason){
        Str reason;
        EXPECT_FALSE(IsValidStringForFont("Arial-Regular", str, reason));
        EXPECT_FALSE(reason.empty());
        EXPECT_EQ(exp_reason, reason);
    };

    test_good("Abcd");
    test_good(" A Q R 024345");

    // Bad font.
    Str fr;
    EXPECT_FALSE(IsValidStringForFont("No-Such-Font", "ABC", fr));
    EXPECT_EQ("Invalid font", fr);

    test_bad("",    "Empty string");
    test_bad(",+*", "Invalid character");
}

TEST_F(TextUtilsTest, GetFontPath) {
    auto path = GetFontPath("Arial-Regular");
    EXPECT_TRUE(path);
    EXPECT_EQ("/fonts/Arial-Regular.ttf", path.ToString());
    EXPECT_FALSE(GetFontPath("NoSUCHFont"));
}

TEST_F(TextUtilsTest, GetTextOutlines) {
    // Use the default font.
    const Str &name = TK::k3DFont;

    // All characters are 1x2 rectangles.
    auto get_rect = [](float llx){
        return Range2f(Point2f(llx, 0), Point2f(llx + 1, 2));
    };

    std::vector<Polygon> polys;

    // 1 character. char_spacing does not matter.
    polys = GetTextOutlines(name, "X", 0, 1);
    ASSERT_EQ(1U, polys.size());
    EXPECT_EQ(1U, polys[0].GetBorderCounts().size());
    EXPECT_EQ(4U, polys[0].GetBorderCounts()[0]);
    EXPECT_EQ(get_rect(0), polys[0].GetBoundingRect());

    // 2 characters. char_spacing of 1.
    polys = GetTextOutlines(name, "XX", 0, 1);
    ASSERT_EQ(2U, polys.size());
    for (int i = 0; i < 2; ++i) {
        EXPECT_EQ(1U, polys[i].GetBorderCounts().size());
        EXPECT_EQ(4U, polys[i].GetBorderCounts()[0]);
        EXPECT_EQ(get_rect(i * 1), polys[i].GetBoundingRect());
    }

    // 3 characters. char_spacing of 2.
    polys = GetTextOutlines(name, "XXX", 0, 2);
    ASSERT_EQ(3U, polys.size());
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(1U, polys[i].GetBorderCounts().size());
        EXPECT_EQ(4U, polys[i].GetBorderCounts()[0]);
        EXPECT_EQ(get_rect(i * 2), polys[i].GetBoundingRect());
    }
}

TEST_F(TextUtilsTest, GetRealTextOutlines) {
    // Use a real FontSystem to test text with holes. Also need a real
    // FileSystem to get the font files.
    UseRealFileSystem(true);
    UseRealFontSystem(true);

    // Single hole ("O"). This should produce a polygon with 2 borders.
    auto polys = GetTextOutlines(TK::k3DFont, "O", 0, 1);
    ASSERT_EQ(1U,  polys.size());
    EXPECT_EQ(2U,  polys[0].GetBorderCounts().size());
    EXPECT_EQ(11U, polys[0].GetBorderCounts()[0]);

    // Two holes ("B"). This should produce a polygon with 3 borders. Use a
    // non-zero complexity to test curve math.
    polys = GetTextOutlines(TK::k3DFont, "B", .4f, 1);
    ASSERT_EQ(1U,  polys.size());
    EXPECT_EQ(3U,  polys[0].GetBorderCounts().size());
    EXPECT_EQ(26U, polys[0].GetBorderCounts()[0]);
}
