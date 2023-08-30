#include "Tests/TestBase.h"
#include "Tests/Testing.h"
#include "Util/FilePath.h"
#include "Util/FontSystem.h"
#include "Util/General.h"
#include "Util/Tuning.h"

/// \ingroup Tests
class FontSystemTest : public TestBase {
  protected:
    FontSystem fs;
};

TEST_F(FontSystemTest, GetAvailableFontNames) {
    const auto names = fs.GetAvailableFontNames();
    EXPECT_FALSE(names.empty());
    EXPECT_TRUE(Util::Contains(names, "Arial-Bold"));
    EXPECT_TRUE(Util::Contains(names, "Arial-Regular"));
    EXPECT_TRUE(Util::Contains(names, "Verdana-Regular"));
}

TEST_F(FontSystemTest, IsValidFontName) {
    EXPECT_TRUE(fs.IsValidFontName("Arial-Regular"));
    EXPECT_TRUE(fs.IsValidFontName("Verdana-Bold"));
    EXPECT_FALSE(fs.IsValidFontName("No-Such-Font"));
}

TEST_F(FontSystemTest, IsValidStringForFont) {
    auto test_good = [&](const Str &str){
        Str reason;
        EXPECT_TRUE(fs.IsValidStringForFont("Arial-Regular", str, reason));
        EXPECT_TRUE(reason.empty());
    };
    auto test_bad = [&](const Str &str, const Str &exp_reason){
        Str reason;
        EXPECT_FALSE(fs.IsValidStringForFont("Arial-Regular", str, reason));
        EXPECT_FALSE(reason.empty());
        EXPECT_EQ(exp_reason, reason);
    };

    test_good("Abcd");
    test_good(" A Q R !?!@#$%^&*()");

    // Bad font.
    Str fr;
    EXPECT_FALSE(fs.IsValidStringForFont("No-Such-Font", "ABC", fr));
    EXPECT_EQ("Invalid font name: No-Such-Font", fr);

    test_bad("", "Empty string");
    test_bad(" \t\n ", "String has only space characters");
    test_bad("¶¡§",
             "String contains invalid character(s) for the font:"
             " [\xC2\xB6\xC2\xA1\xC2\xA7]");
}

TEST_F(FontSystemTest, GetFontPath) {
    auto path = fs.GetFontPath("Arial-Regular");
    EXPECT_TRUE(path);
    EXPECT_TRUE(path.ToString().contains("Arial"));
    EXPECT_FALSE(fs.GetFontPath("NoSUCHFont"));
}
