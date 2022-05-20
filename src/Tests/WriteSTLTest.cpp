#include <vector>

#include "IO/STLWriter.h"
#include "Items/UnitConversion.h"
#include "Models/BoxModel.h"
#include "Models/RootModel.h"
#include "Models/TextModel.h"
#include "SceneTestBase.h"
#include "Selection.h"
#include "Testing.h"
#include "Util/String.h"

class WriteSTLTest : public SceneTestBase {
  protected:
    // Writes the given Model in the given format with the given
    // UnitConversion, returning the string representing the contents of the
    // output file.
    std::string WriteConvModelAsSTL(const ModelPtr &model,
                                    const UnitConversion &conv,
                                    FileFormat format) {
        // Construct a Selection with the Model.
        auto root_model = Model::CreateModel<RootModel>();
        root_model->AddChildModel(model);
        SelPath sel_path(root_model, model);
        Selection sel(sel_path);

        TempFile tmp("");
        EXPECT_TRUE(WriteSTLFile(sel, tmp.GetPath(), format, conv));

        return tmp.GetContents();
    }

    // Same as above, but uses the default (identity) UnitConversion.
    std::string WriteModelAsSTL(const ModelPtr &model, FileFormat format) {
        UnitConversionPtr conv = GetDefaultUC();
        return WriteConvModelAsSTL(model, *conv, format);
    }
};

TEST_F(WriteSTLTest, TextBox) {
    // Create a Box from -2 to +2 in X, -3 to +3 in Y, and -4 to +4 in Z.
    // (Different sizes to check the coordinate system conversions.)
    auto box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(2, 3, 4));

    const std::string expected = ReadDataFile("exportedTextBox.stl");
    const std::string actual   = WriteModelAsSTL(box, FileFormat::kTextSTL);
    EXPECT_TRUE(CompareStrings(expected, actual));
}

TEST_F(WriteSTLTest, BinaryBox) {
    // Create a Box from -2 to +2 in X, -3 to +3 in Y, and -4 to +4 in Z.
    // (Different sizes to check the coordinate system conversions.)
    auto box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(2, 3, 4));

    const std::string expected = ReadDataFile("exportedBinaryBox.stl");
    const std::string actual   = WriteModelAsSTL(box, FileFormat::kBinarySTL);

    // Compare as binary strings.
    ASSERT_EQ(expected.size(), actual.size());
    for (size_t i = 0; i < expected.size(); ++i)
        ASSERT_EQ(expected[i], actual[i]) << "Bytes differ at index " << i;
}

TEST_F(WriteSTLTest, TextBoxConv) {
    // Use a larger scale, which should be counteracted by conversion.
    auto box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(2000, 3000, 4000));

    // Converting from millimeters to meters should change the scales back to
    // 2,3,4.
    UnitConversionPtr conv = GetDefaultUC();
    conv->SetFromUnits(UnitConversion::Units::kMillimeters);
    conv->SetToUnits(UnitConversion::Units::kMeters);

    const std::string expected = ReadDataFile("exportedTextBox.stl");
    const std::string actual   = WriteConvModelAsSTL(box, *conv,
                                                     FileFormat::kTextSTL);
    EXPECT_TRUE(CompareStrings(expected, actual));
}

TEST_F(WriteSTLTest, TextF) {
    // Text "F" to make sure STL orientation is correct.
    auto f = Model::CreateModel<TextModel>();
    f->SetUniformScale(10);
    f->SetTextString("F");

    const std::string expected = ReadDataFile("exportedTextF.stl");
    const std::string actual   = WriteModelAsSTL(f, FileFormat::kTextSTL);
    EXPECT_TRUE(CompareStrings(expected, actual));
}

