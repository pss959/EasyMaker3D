#include <ion/math/transformutils.h>

#include "Base/Tuning.h"
#include "Math/TextUtils.h"
#include "Util/FilePath.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

class TextUtilsTest : public TestBase {
  protected:
    void TestPoly(const Polygon &poly, std::vector<size_t> border_counts) {
        EXPECT_EQ(border_counts.size(), poly.GetBorderCounts().size());
        for (size_t i = 0; i < border_counts.size(); ++i)
            EXPECT_EQ(border_counts[i], poly.GetBorderCounts()[i]);
    }
};

TEST_F(TextUtilsTest, IsValidFontName) {
    EXPECT_TRUE(IsValidFontName("Arial-Regular"));
    EXPECT_TRUE(IsValidFontName("Verdana-Bold"));
    EXPECT_FALSE(IsValidFontName("No-Such-Font"));
}

TEST_F(TextUtilsTest, IsValidStringForFont) {
    auto test_good = [](const std::string &str){
        std::string reason;
        EXPECT_TRUE(IsValidStringForFont("Arial-Regular", str, reason));
        EXPECT_TRUE(reason.empty());
    };
    auto test_bad = [](const std::string &str, const std::string &exp_reason){
        std::string reason;
        EXPECT_FALSE(IsValidStringForFont("Arial-Regular", str, reason));
        EXPECT_FALSE(reason.empty());
        EXPECT_EQ(exp_reason, reason);
    };

    test_good("Abcd");
    test_good(" A Q R !?!@#$%^&*()");

    // Bad font.
    std::string fr;
    EXPECT_FALSE(IsValidStringForFont("No-Such-Font", "ABC", fr));
    EXPECT_EQ("Invalid font name: No-Such-Font", fr);

    test_bad("", "Empty string");
    test_bad(" \t\n ", "String has only space characters");
    test_bad("¶¡§",
             "String contains invalid character(s) for the font:"
             " [\xC2\xB6\xC2\xA1\xC2\xA7]");
}

TEST_F(TextUtilsTest, SingleCharOutlines) {
    // Use the default font.
    const std::string &name = TK::k3DFont;

    std::vector<Polygon> polys;

    // One upper-case 'T'. Results are 1 outer border with 8 points.
    polys = GetTextOutlines(name, "T", 0, 1);
    ASSERT_EQ(1U, polys.size());
    TestPoly(polys[0], std::vector<size_t>{8});

    // One upper-case 'O' with complexity = 0. Results are 1 outer border
    // with 11 points and 1 inner border with 9 points.
    polys = GetTextOutlines(name, "O", 0, 1);
    ASSERT_EQ(1U, polys.size());
    TestPoly(polys[0], std::vector<size_t>{11, 9});

    // Repeat with default complexity. Results are 1 outer border
    // with 27 points and 1 inner border with 24 points.
    polys = GetTextOutlines(name, "O", TK::kModelComplexity, 1);
    ASSERT_EQ(1U, polys.size());
    TestPoly(polys[0], std::vector<size_t>{27, 24});

    // Repeat with upper-case 'Q'. This used to result in an invalid mesh
    // when text was created.
    polys = GetTextOutlines(name, "Q", TK::kModelComplexity, 1);
    ASSERT_EQ(1U, polys.size());
    TestPoly(polys[0], std::vector<size_t>{32, 28});
}

TEST_F(TextUtilsTest, TwoCharOutlines) {
    // Use the default font.
    const std::string &name = TK::k3DFont;

    std::vector<Polygon> polys;

    // Two upper-case 'T's. Results are 2 polygons, each with 1 outer border
    // with 8 points.
    polys = GetTextOutlines(name, "TT", 0, 1);
    ASSERT_EQ(2U, polys.size());
    TestPoly(polys[0], std::vector<size_t>{8});
    TestPoly(polys[1], std::vector<size_t>{8});

    // Character spacing test. First determine what the default space between
    // the two T's is.
    const Range2f rect0 = polys[0].GetBoundingRect();
    const Range2f rect1 = polys[1].GetBoundingRect();
    const float space = rect1.GetCenter()[0] - rect0.GetCenter()[0];
    EXPECT_LT(0, space);

    // Lay out the text again with double the character spacing. Everything
    // should be the same except for the space between the characters (distance
    // between the centers).
    polys = GetTextOutlines(name, "TT", 0, 2);
    ASSERT_EQ(2U, polys.size());
    TestPoly(polys[0], std::vector<size_t>{8});
    TestPoly(polys[1], std::vector<size_t>{8});
    const Range2f new_rect0 = polys[0].GetBoundingRect();
    const Range2f new_rect1 = polys[1].GetBoundingRect();
    const float new_space = new_rect1.GetCenter()[0] - new_rect0.GetCenter()[0];
    EXPECT_EQ(rect0.GetSize(), new_rect0.GetSize());
    EXPECT_EQ(rect1.GetSize(), new_rect1.GetSize());
    EXPECT_EQ(2 * space, new_space);
}
