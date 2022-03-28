#include <ion/math/transformutils.h>

#include "Math/TextUtils.h"
#include "Util/FilePath.h"
#include "TestBase.h"
#include "Testing.h"

class TextUtilsTest : public TestBase {
};

TEST_F(TextUtilsTest, GetFontDesc) {
    FilePath path = GetDataPath("Fonts/Ubuntu-L.ttf");
    EXPECT_EQ("Ubuntu-Light", GetFontDesc(path));

    path = GetDataPath("Fonts/calibrib.ttf");
    EXPECT_EQ("Calibri-Bold", GetFontDesc(path));

    path = "/no/such/font.ttf";
    EXPECT_EQ("", GetFontDesc(path));
}
