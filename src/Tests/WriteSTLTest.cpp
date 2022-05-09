#include <vector>

#include "IO/STLWriter.h"
#include "Models/BoxModel.h"
#include "SceneTestBase.h"
#include "Testing.h"
#include "UnitConversion.h"
#include "Util/String.h"

class WriteSTLTest : public SceneTestBase {
};

TEST_F(WriteSTLTest, TextBox) {
    // Create a Box from -2 to +2 in X, -3 to +3 in Y, and -4 to +4 in Z.
    // (Different sizes to check the coordinate system conversions.)
    auto box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(2, 3, 4));

    TempFile tmp("");
    UnitConversionPtr conv = GetDefaultUC();
    WriteSTLFile(std::vector<ModelPtr>(1, box), tmp.GetPath(),
                 FileFormat::kTextSTL, *conv);

    const std::string expected = ReadDataFile("exportedTextBox.stl");
    const std::string actual   = tmp.GetContents();
    EXPECT_TRUE(CompareStrings(expected, actual));
}
