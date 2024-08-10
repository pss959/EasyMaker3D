#include <vector>

#include "IO/STLWriter.h"
#include "Items/UnitConversion.h"
#include "Math/MeshUtils.h"
#include "Models/BoxModel.h"
#include "Models/TextModel.h"
#include "Tests/SceneTestBase.h"
#include "Tests/TempFile.h"
#include "Tests/Testing.h"
#include "Util/String.h"

/// \ingroup Tests
class WriteSTLTest : public SceneTestBase {
  protected:
    /// Returns a default UnitConversion instance.
    static UnitConversionPtr GetDefaultUC() {
        return CreateObject<UnitConversion>();
    }

    /// Writes the given Model in the given format with the given
    /// UnitConversion, returning the string representing the contents of the
    /// output file.
    Str WriteConvModelAsSTL(const ModelPtr &model, const UnitConversion &conv,
                            FileFormat format) {
        const TriMesh mesh = TransformMesh(model->GetMesh(),
                                           model->GetModelMatrix());

        TempFile tmp("");
        EXPECT_TRUE(WriteSTLFile(std::vector<TriMesh>{ mesh },
                                 tmp.GetPath(), format, conv.GetFactor()));

        return tmp.GetContents();
    }

    // Same as WriteConvModelAsSTL(), but uses the default (identity)
    // UnitConversion.
    Str WriteModelAsSTL(const ModelPtr &model, FileFormat format) {
        UnitConversionPtr conv = GetDefaultUC();
        return WriteConvModelAsSTL(model, *conv, format);
    }
};

TEST_F(WriteSTLTest, TextBox) {
    // Create a Box from -2 to +2 in X, -3 to +3 in Y, and -4 to +4 in Z.
    // (Different sizes to check the coordinate system conversions.)
    auto box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(2, 3, 4));

    const Str expected = ReadDataFile("exportedTextBox.stl");
    const Str actual   = WriteModelAsSTL(box, FileFormat::kTextSTL);
    EXPECT_TRUE(CompareStrings(expected, actual));
}

TEST_F(WriteSTLTest, BinaryBox) {
    // Create a Box from -2 to +2 in X, -3 to +3 in Y, and -4 to +4 in Z.
    // (Different sizes to check the coordinate system conversions.)
    auto box = Model::CreateModel<BoxModel>();
    box->SetScale(Vector3f(2, 3, 4));

    const Str expected = ReadDataFile("exportedBinaryBox.stl");
    const Str actual   = WriteModelAsSTL(box, FileFormat::kBinarySTL);

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

    const Str expected = ReadDataFile("exportedTextBox.stl");
    const Str actual   = WriteConvModelAsSTL(box, *conv,
                                                     FileFormat::kTextSTL);
    EXPECT_TRUE(CompareStrings(expected, actual));
}

TEST_F(WriteSTLTest, TextF) {
    // Text "F" to make sure STL orientation is correct.
    UseRealFontSystem(true);
    auto f = Model::CreateModel<TextModel>();
    f->SetUniformScale(10);
    f->SetTextString("F");

    const Str expected = ReadDataFile("exportedTextF.stl");
    const Str actual   = WriteModelAsSTL(f, FileFormat::kTextSTL);
    EXPECT_TRUE(CompareStrings(expected, actual));
}

TEST_F(WriteSTLTest, Errors) {
    auto box  = Model::CreateModel<BoxModel>();
    auto conv = GetDefaultUC();

    EXPECT_FALSE(WriteSTLFile(std::vector<TriMesh>{ box->GetMesh() },
                              "/no/such/path/to/file.stl",
                              FileFormat::kTextSTL, conv->GetFactor()));
}
