#include <vector>

#include "IO/STLWriter.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "SceneTestBase.h"
#include "Selection.h"
#include "Testing.h"
#include "UnitConversion.h"
#include "Util/String.h"

class WriteSTLTest : public SceneTestBase {
};

// XXXX Clean this up!

TEST_F(WriteSTLTest, TextBox) {
    // XXXX Put this somewhere useful...
    auto root_model = Model::CreateModel<RootModel>();

    // Create a Box from -2 to +2 in X, -3 to +3 in Y, and -4 to +4 in Z.
    // (Different sizes to check the coordinate system conversions.)
    auto box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(2, 3, 4));

    root_model->AddChildModel(box);

    TempFile tmp("");
    UnitConversionPtr conv = GetDefaultUC();

    // Construct a Selection with the box.
    SelPath sel_path(root_model, box);
    Selection sel(sel_path);

    EXPECT_TRUE(WriteSTLFile(sel, tmp.GetPath(), FileFormat::kTextSTL, *conv));

    const std::string expected = ReadDataFile("exportedTextBox.stl");
    const std::string actual   = tmp.GetContents();
    EXPECT_TRUE(CompareStrings(expected, actual));
}

TEST_F(WriteSTLTest, BinaryBox) {
    // XXXX Put this somewhere useful...
    auto root_model = Model::CreateModel<RootModel>();

    // Create a Box from -2 to +2 in X, -3 to +3 in Y, and -4 to +4 in Z.
    // (Different sizes to check the coordinate system conversions.)
    auto box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(2, 3, 4));

    root_model->AddChildModel(box);

    TempFile tmp("");
    UnitConversionPtr conv = GetDefaultUC();

    // Construct a Selection with the box.
    SelPath sel_path(root_model, box);
    Selection sel(sel_path);

    EXPECT_TRUE(WriteSTLFile(sel, tmp.GetPath(),
                             FileFormat::kBinarySTL, *conv));

    const std::string expected = ReadDataFile("exportedBinaryBox.stl");
    const std::string actual   = tmp.GetContents();

    // Compare as binary strings.
    ASSERT_EQ(expected.size(), actual.size());
    for (size_t i = 0; i < expected.size(); ++i)
        ASSERT_EQ(expected[i], actual[i]) << "Bytes differ at index " << i;
}
