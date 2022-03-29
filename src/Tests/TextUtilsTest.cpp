#include <ion/math/transformutils.h>

#include "Defaults.h"
#include "Math/TextUtils.h"
#include "Util/FilePath.h"
#include "TestBase.h"
#include "Testing.h"

class TextUtilsTest : public TestBase {
  protected:
    void TestPoly(const Polygon &poly, std::vector<size_t> border_counts) {
        EXPECT_EQ(border_counts.size(), poly.GetBorderCounts().size());
        for (size_t i = 0; i < border_counts.size(); ++i)
            EXPECT_EQ(border_counts[i], poly.GetBorderCounts()[i]);
    }
};

TEST_F(TextUtilsTest, GetFontDesc) {
    EXPECT_EQ("Arial-Regular", GetFontDesc(GetFontPath("Arial")));

    EXPECT_EQ("Verdana-Bold", GetFontDesc(GetFontPath("Verdana_Bold")));

    EXPECT_EQ("", GetFontDesc(GetFontPath("no-such-font")));
}

TEST_F(TextUtilsTest, SingleCharOutlines) {
    // Use the default font.
    const FilePath path = GetFontPath(Defaults::kFontName);

    std::vector<Polygon> polys;

    // One upper-case 'T'. Results are 1 outer border with 8 points.
    polys = GetTextOutlines(path, "T", 0, 1);
    ASSERT_EQ(1U, polys.size());
    TestPoly(polys[0], std::vector<size_t>{8});

    // One upper-case 'O' with complexity = 0. Results are 1 outer border
    // with 11 points and 1 inner border with 9 points.
    polys = GetTextOutlines(path, "O", 0, 1);
    ASSERT_EQ(1U, polys.size());
    TestPoly(polys[0], std::vector<size_t>{11, 9});

    // Repeat with default complexity. Results are 1 outer border
    // with 27 points and 1 inner border with 24 points.
    polys = GetTextOutlines(path, "O", Defaults::kModelComplexity, 1);
    ASSERT_EQ(1U, polys.size());
    TestPoly(polys[0], std::vector<size_t>{27, 24});

    // Repeat with upper-case 'Q'. This used to result in an invalid mesh
    // when text was created.
    polys = GetTextOutlines(path, "Q", Defaults::kModelComplexity, 1);
    ASSERT_EQ(1U, polys.size());
    TestPoly(polys[0], std::vector<size_t>{32, 28});
}

TEST_F(TextUtilsTest, TwoCharOutlines) {
    // Use the default font.
    const FilePath path = GetFontPath(Defaults::kFontName);

    std::vector<Polygon> polys;

    // Two upper-case 'T's. Results are 2 polygons, each with 1 outer border
    // with 8 points.
    polys = GetTextOutlines(path, "TT", 0, 1);
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
    polys = GetTextOutlines(path, "TT", 0, 2);
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
