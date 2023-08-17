#include <ion/math/angleutils.h>

#include "Math/ColorRing.h"
#include "Math/Linear.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"

/// \ingroup Tests
class ColorRingTest : public TestBase {
  protected:
    /// Returns a point within the ColorRing.
    static Point2f GetPoint_(float rad_frac, float deg) {
        const float r = Lerp(rad_frac,
                             TK::kColorRingInnerRadius,
                             TK::kColorRingOuterRadius);
        const Anglef angle = Anglef::FromDegrees(deg);
        return Point2f( r * ion::math::Cosine(angle),
                       -r * ion::math::Sine(angle));
    }

    /// Returns the color as a hex string for a point within the ColorRing.
    static Str GetString_(float rad_frac, float deg) {
        const auto pt = GetPoint_(rad_frac, deg);
        return ColorRing::GetColorForPoint(pt).ToHexString();
    }
};

TEST_F(ColorRingTest, GetColorForPoint) {
    // Use hex strings to avoid floating point errors.
    EXPECT_EQ("#e6acacff", GetString_(0, 0));
    EXPECT_EQ("#e6e6acff", GetString_(0, 60));
    EXPECT_EQ("#ace6acff", GetString_(0, 120));
    EXPECT_EQ("#ace6e6ff", GetString_(0, 180));
    EXPECT_EQ("#acace6ff", GetString_(0, 240));
    EXPECT_EQ("#e6ace6ff", GetString_(0, 300));
    EXPECT_EQ("#f27979ff", GetString_(1, 0));
    EXPECT_EQ("#f2f279ff", GetString_(1, 60));
    EXPECT_EQ("#79f279ff", GetString_(1, 120));
    EXPECT_EQ("#79f2f2ff", GetString_(1, 180));
    EXPECT_EQ("#7979f2ff", GetString_(1, 240));
    EXPECT_EQ("#f279f2ff", GetString_(1, 300));
}

TEST_F(ColorRingTest, GetPointForColor) {
    // Reverse of above.
    const auto to_point = [](const Str &hex){
        Color c;
        EXPECT_TRUE(c.FromHexString(hex));
        return ColorRing::GetPointForColor(c);
    };

// Macro for testing points with a larger tolerance.
#define EXPECT_CPTS_CLOSE_(EXP, P) EXPECT_PRED3(PointsClose2T, EXP, P, .025f)

    EXPECT_CPTS_CLOSE_(GetPoint_(0, 0),   to_point("#e6acacff"));
    EXPECT_CPTS_CLOSE_(GetPoint_(0, 60),  to_point("#e6e6acff"));
    EXPECT_CPTS_CLOSE_(GetPoint_(0, 120), to_point("#ace6acff"));
    EXPECT_CPTS_CLOSE_(GetPoint_(0, 180), to_point("#ace6e6ff"));
    EXPECT_CPTS_CLOSE_(GetPoint_(0, 240), to_point("#acace6ff"));
    EXPECT_CPTS_CLOSE_(GetPoint_(0, 300), to_point("#e6ace6ff"));
    EXPECT_CPTS_CLOSE_(GetPoint_(1, 0),   to_point("#f27979ff"));
    EXPECT_CPTS_CLOSE_(GetPoint_(1, 60),  to_point("#f2f279ff"));
    EXPECT_CPTS_CLOSE_(GetPoint_(1, 120), to_point("#79f279ff"));
    EXPECT_CPTS_CLOSE_(GetPoint_(1, 180), to_point("#79f2f2ff"));
    EXPECT_CPTS_CLOSE_(GetPoint_(1, 240), to_point("#7979f2ff"));
    EXPECT_CPTS_CLOSE_(GetPoint_(1, 300), to_point("#f279f2ff"));

#undef EXPECT_CPTS_CLOSE_
}
