//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Polygon.h"
#include "Math/PolygonBuilder.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"
#include "Util/FilePath.h"
#include "Util/FontSystem.h"
#include "Util/General.h"
#include "Util/Tuning.h"

/// \ingroup Tests
class FontSystemTest : public TestBase {
  protected:
    /// Use a real FontSystem for these tests.
    FontSystemPtr fs;

    FontSystemTest() {
        /// Use a real FontSystem and make it easily accessible in tests. Also
        /// have to use a real FileSystem to read the fonts.
        UseRealFileSystem(true);
        UseRealFontSystem(true);
        fs = FontSystem::GetInstalled();
        EXPECT_FALSE(fs->IsFake());
    }

    /// Uses a PolygonBuilder to create Polygon instances representing text
    /// outlines using FontSystem::GetTextOutlines() for the default
    /// font. Returns the Polygon vector.
    std::vector<Polygon> GetTextPolygons(const Str &text_str,
                                         float complexity,
                                         float char_spacing) {
        PolygonBuilder builder;
        fs->GetTextOutlines(TK::k3DFont, text_str, char_spacing,
                            builder.SetUpForText());
        std::vector<Polygon> polys;
        builder.AddPolygons(polys, complexity);
        return polys;
    }

    /// Verifies that the given Polygon's border counts match \p border_counts.
    void TestPoly(const Polygon &poly, std::vector<size_t> border_counts) {
        EXPECT_EQ(border_counts.size(), poly.GetBorderCounts().size());
        for (size_t i = 0; i < border_counts.size(); ++i)
            EXPECT_EQ(border_counts[i], poly.GetBorderCounts()[i]);
    }
};

TEST_F(FontSystemTest, GetAvailableFontNames) {
    const auto names = fs->GetAvailableFontNames();
    EXPECT_FALSE(names.empty());
    EXPECT_TRUE(Util::Contains(names, "Arial-Bold"));
    EXPECT_TRUE(Util::Contains(names, "Arial-Regular"));
    EXPECT_TRUE(Util::Contains(names, "Verdana-Regular"));
}

TEST_F(FontSystemTest, IsValidFontName) {
    EXPECT_TRUE(fs->IsValidFontName("Arial-Regular"));
    EXPECT_TRUE(fs->IsValidFontName("Verdana-Bold"));
    EXPECT_FALSE(fs->IsValidFontName("No-Such-Font"));
}

TEST_F(FontSystemTest, IsValidStringForFont) {
    auto test_good = [&](const Str &str){
        Str reason;
        EXPECT_TRUE(fs->IsValidStringForFont("Arial-Regular", str, reason));
        EXPECT_TRUE(reason.empty());
    };
    auto test_bad = [&](const Str &str, const Str &exp_reason){
        Str reason;
        EXPECT_FALSE(fs->IsValidStringForFont("Arial-Regular", str, reason));
        EXPECT_FALSE(reason.empty());
        EXPECT_EQ(exp_reason, reason);
    };

    test_good("Abcd");
    test_good(" A Q R !?!@#$%^&*()");

    // Bad font.
    Str fr;
    EXPECT_FALSE(fs->IsValidStringForFont("No-Such-Font", "ABC", fr));
    EXPECT_EQ("Invalid font name: No-Such-Font", fr);

    test_bad("", "Empty string");
    test_bad(" \t\n ", "String has only space characters");
    test_bad("¶¡§",
             "String contains invalid character(s) for the font:"
             " [\xC2\xB6\xC2\xA1\xC2\xA7]");
}

TEST_F(FontSystemTest, GetFontPath) {
    auto path = fs->GetFontPath("Arial-Regular");
    EXPECT_TRUE(path);
    EXPECT_TRUE(path.ToString().contains("Arial"));
    EXPECT_FALSE(fs->GetFontPath("NoSUCHFont"));
}

TEST_F(FontSystemTest, SingleCharOutlines) {
    std::vector<Polygon> polys;

    // One upper-case 'T'. Results are 1 outer border with 8 points.
    polys = GetTextPolygons("T", 0, 1);
    ASSERT_EQ(1U, polys.size());
    TestPoly(polys[0], std::vector<size_t>{8});

    // One upper-case 'O' with complexity = 0. Results are 1 outer border
    // with 11 points and 1 inner border with 9 points.
    polys = GetTextPolygons("O", 0, 1);
    ASSERT_EQ(1U, polys.size());
    TestPoly(polys[0], std::vector<size_t>{11, 9});

    // Repeat with default complexity. Results are 1 outer border
    // with 27 points and 1 inner border with 24 points.
    polys = GetTextPolygons("O", TK::kModelComplexity, 1);
    ASSERT_EQ(1U, polys.size());
    TestPoly(polys[0], std::vector<size_t>{27, 24});

    // Repeat with upper-case 'Q'. This used to result in an invalid mesh
    // when text was created.
    polys = GetTextPolygons("Q", TK::kModelComplexity, 1);
    ASSERT_EQ(1U, polys.size());
    TestPoly(polys[0], std::vector<size_t>{32, 28});
}

TEST_F(FontSystemTest, TwoCharOutlines) {
    std::vector<Polygon> polys;

    // Two upper-case 'T's. Results are 2 polygons, each with 1 outer border
    // with 8 points.
    polys = GetTextPolygons("TT", 0, 1);
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
    polys = GetTextPolygons("TT", 0, 2);
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
